// bench.cpp - đo runtime 5 thuật toán trên toàn bộ dataset (*.inp / *.out), có time limit
//
//   ./bench [--in dataset/input] [--out dataset/output] [--csv benchmark.csv]
//           [--repeat 3] [--timeout 180] [--skip-after-tle]
//
//   --timeout         time limit (giây) cho MỖI lần chạy của 1 thuật toán, mặc định 180 (= 3 phút)
//   --repeat          số lần chạy mỗi (file, thuật toán); lấy trung bình và nhỏ nhất
//   --skip-after-tle  (tuỳ chọn) nếu thuật toán đã TLE ở file có n = n0 thì các file sau có n >= n0
//                     được đánh dấu TLE* luôn mà không chạy lại -> tiết kiệm rất nhiều thời gian
//
// Trên Linux/macOS: mỗi lần sort chạy trong 1 tiến trình con (fork) và bị kill khi quá giờ.
// Trên Windows    : chạy trong thread; thread quá giờ không kill được nên vẫn tốn CPU đến khi bench kết thúc
//                   (nên dùng WSL / --skip-after-tle nếu muốn đo chính xác).
#include "sort_common.hpp"

#include <cerrno>
#include <cmath>
#include <iomanip>

#if !defined(_WIN32) && !defined(BENCH_NO_FORK)
#define BENCH_USE_FORK 1
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#endif

enum class Status { OK, WRONG, TLE, ERR };

struct RunOut {
    Status st = Status::ERR;
    double ms = 0;
};

#ifdef BENCH_USE_FORK
// ---------- Linux / macOS: fork + poll + kill ----------
static RunOut runLimited(int algo, const vector<Int>& data, const vector<Int>& expected, double limitMs) {
    struct Msg {
        double ms;
        int ok;
    };
    int fd[2];
    if (pipe(fd) != 0) return {Status::ERR, 0};

    fflush(stdout);  // tránh tiến trình con nhân đôi buffer stdout
    pid_t pid = fork();
    if (pid < 0) {
        close(fd[0]);
        close(fd[1]);
        return {Status::ERR, 0};
    }

    if (pid == 0) {  // ----- tiến trình con -----
        close(fd[0]);
        vector<Int> work = data;  // copy nằm ngoài đoạn đo thời gian
        auto t0 = chrono::steady_clock::now();
        runSort(algo, work);
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
        Msg msg{ms, work == expected ? 1 : 0};
        ssize_t w = write(fd[1], &msg, sizeof msg);
        (void)w;
        _exit(0);
    }

    // ----- tiến trình cha -----
    close(fd[1]);
    RunOut res{Status::ERR, 0};
    auto start = chrono::steady_clock::now();
    pollfd pfd{fd[0], POLLIN, 0};
    int pr;
    do {
        double elapsed = chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
        int remaining = (int)ceil(limitMs - elapsed);
        if (remaining <= 0) {
            pr = 0;
            break;
        }
        pr = poll(&pfd, 1, remaining);
    } while (pr < 0 && errno == EINTR);

    if (pr > 0) {
        Msg msg;
        ssize_t r = read(fd[0], &msg, sizeof msg);
        if (r == (ssize_t)sizeof msg) {
            res.ms = msg.ms;
            res.st = msg.ok ? Status::OK : Status::WRONG;
            if (res.ms > limitMs) res.st = Status::TLE;
        }  // r <= 0: con bị crash (vd. tràn stack) -> ERR
    } else if (pr == 0) {
        res = {Status::TLE, limitMs};
    }

    kill(pid, SIGKILL);  // an toàn: nếu con đã xong thì chỉ là zombie chưa được wait
    close(fd[0]);
    waitpid(pid, nullptr, 0);
    return res;
}
#else
// ---------- Windows (hoặc -DBENCH_NO_FORK): thread + wait_for ----------
struct Shared {
    mutex m;
    condition_variable cv;
    bool done = false, ok = false;
    double ms = 0;
};

static RunOut runLimited(int algo, const vector<Int>& data, const vector<Int>& expected, double limitMs) {
    auto sh = make_shared<Shared>();
    auto d = make_shared<vector<Int>>(data);
    auto e = make_shared<vector<Int>>(expected);
    thread([=]() {
        vector<Int> work = *d;
        auto t0 = chrono::steady_clock::now();
        runSort(algo, work);
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
        bool ok = (work == *e);
        lock_guard<mutex> lk(sh->m);
        sh->ms = ms;
        sh->ok = ok;
        sh->done = true;
        sh->cv.notify_all();
    }).detach();

    unique_lock<mutex> lk(sh->m);
    if (!sh->cv.wait_for(lk, chrono::duration<double, milli>(limitMs), [&] { return sh->done; }))
        return {Status::TLE, limitMs};
    RunOut res{sh->ok ? Status::OK : Status::WRONG, sh->ms};
    if (res.ms > limitMs) res.st = Status::TLE;
    return res;
}
#endif

// ---------- Cấu hình & kết quả ----------
struct Options {
    fs::path inDir = "dataset/input";
    fs::path outDir = "dataset/output";
    int repeat = 3;
    double timeoutSec = 180.0;  // 3 phút
    bool skipAfterTle = false;
    string csv = "benchmark.csv";
};

struct Result {
    Status st = Status::OK;
    bool predicted = false;  // TLE suy ra từ --skip-after-tle, không thực sự chạy
    double avgMs = 0, minMs = 0;
};

static Options parseArgs(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; i++) {
        string k = argv[i];
        if (k == "--skip-after-tle") {
            o.skipAfterTle = true;
            continue;
        }
        if (i + 1 >= argc) {
            cerr << "Thieu gia tri cho " << k << '\n';
            break;
        }
        string v = argv[++i];
        if (k == "--in") o.inDir = v;
        else if (k == "--out") o.outDir = v;
        else if (k == "--repeat") o.repeat = max(1, atoi(v.c_str()));
        else if (k == "--timeout") o.timeoutSec = max(0.001, atof(v.c_str()));
        else if (k == "--csv") o.csv = v;
        else cerr << "Bo qua tham so khong hop le: " << k << '\n';
    }
    return o;
}

int runBenchmark(int argc, char** argv) {
    ios::sync_with_stdio(false);
    Options opt = parseArgs(argc, argv);
    const double limitMs = opt.timeoutSec * 1000.0;

    if (!fs::exists(opt.inDir)) {
        cerr << "Khong tim thay thu muc " << opt.inDir << '\n';
        return 1;
    }
    vector<fs::path> files;
    for (auto& e : fs::directory_iterator(opt.inDir))
        if (e.is_regular_file() && e.path().extension() == ".inp") files.push_back(e.path());
    sort(files.begin(), files.end());
    if (files.empty()) {
        cerr << "Khong co file .inp nao trong " << opt.inDir << '\n';
        return 1;
    }

    fs::path csvPath(opt.csv);
    if (csvPath.has_parent_path()) fs::create_directories(csvPath.parent_path());
    ofstream csv(csvPath);
    if (!csv) {
        cerr << "Khong mo duoc file CSV " << csvPath << '\n';
        return 1;
    }
    csv << fixed << setprecision(6);
    csv << "file,n,algorithm,avg_ms,min_ms,status\n";

    printf("Time limit: %g s / lan chay | repeat: %d%s\n\n", opt.timeoutSec, opt.repeat,
           opt.skipAfterTle ? " | skip-after-tle: on" : "");
    printf("%-12s %10s", "file", "n");
    for (int a = 1; a <= NUM_ALGOS; a++) printf(" %12s", algoName(a));
    printf("   (ms, trung binh)\n");

    double total[NUM_ALGOS + 1] = {0};
    int okCount[NUM_ALGOS + 1] = {0}, tleCount[NUM_ALGOS + 1] = {0};
    size_t tleAt[NUM_ALGOS + 1];
    fill(tleAt, tleAt + NUM_ALGOS + 1, (size_t)-1);
    int failures = 0;

    for (auto& f : files) {
        ifstream fin(f);
        vector<Int> data;
        if (!fin || !readArray(fin, data)) {
            cerr << "Bo qua " << f << " (khong doc duoc)\n";
            continue;
        }
        const size_t n = data.size();

        // Kết quả mong đợi: ưu tiên file .out có sẵn, nếu không có thì dùng std::sort
        vector<Int> expected;

        printf("%-12s %10zu", f.filename().string().c_str(), n);
        fflush(stdout);

        for (int algo = 1; algo <= NUM_ALGOS; algo++) {
            Result r;
            fs::path outPath = opt.outDir / algoDir(algo) / f.filename();
            outPath.replace_extension(".out");
            if (!readExpected(outPath, n, expected)) {
                expected = data;
                sort(expected.begin(), expected.end());
                if (!ASCENDING) reverse(expected.begin(), expected.end());
            }

            if (opt.skipAfterTle && n >= tleAt[algo]) {
                r.st = Status::TLE;
                r.predicted = true;
            } else {
                double sum = 0, best = 1e300;
                for (int rep = 0; rep < opt.repeat; rep++) {
                    RunOut o = runLimited(algo, data, expected, limitMs);
                    if (o.st == Status::TLE || o.st == Status::ERR) {
                        r.st = o.st;
                        break;  // đã TLE/crash thì không chạy lặp nữa
                    }
                    if (o.st == Status::WRONG) r.st = Status::WRONG;
                    sum += o.ms;
                    best = min(best, o.ms);
                    r.avgMs = sum / (rep + 1);
                    r.minMs = best;
                }
                if (r.st == Status::TLE) tleAt[algo] = min(tleAt[algo], n);
            }

            switch (r.st) {
                case Status::OK:
                    total[algo] += r.avgMs;
                    okCount[algo]++;
                    break;
                case Status::TLE: tleCount[algo]++; break;
                default: failures++; break;
            }

            char cell[32];
            switch (r.st) {
                case Status::OK: snprintf(cell, sizeof cell, "%.3f", r.avgMs); break;
                case Status::WRONG: snprintf(cell, sizeof cell, "FAIL"); break;
                case Status::TLE: snprintf(cell, sizeof cell, r.predicted ? "TLE*" : "TLE"); break;
                case Status::ERR: snprintf(cell, sizeof cell, "ERR"); break;
            }
            printf(" %12s", cell);
            fflush(stdout);

            const char* stName = r.st == Status::OK ? "ok" : r.st == Status::WRONG ? "FAIL"
                               : r.st == Status::ERR ? "ERR" : (r.predicted ? "TLE_predicted" : "TLE");
            csv << f.filename().string() << ',' << n << ',' << algoName(algo) << ',';
            if (r.st == Status::OK || r.st == Status::WRONG) csv << r.avgMs << ',' << r.minMs;
            else csv << ',';
            csv << ',' << stName << '\n';
            csv.flush();
        }
        printf("\n");
    }

    printf("\n%-23s", "Tong thoi gian (ms)");
    for (int a = 1; a <= NUM_ALGOS; a++) printf(" %12.3f", total[a]);
    printf("\n%-23s", "So file OK");
    for (int a = 1; a <= NUM_ALGOS; a++) printf(" %12d", okCount[a]);
    printf("\n%-23s", "So file TLE");
    for (int a = 1; a <= NUM_ALGOS; a++) printf(" %12d", tleCount[a]);
    printf("\n\n(TLE* = suy ra tu --skip-after-tle, khong chay thuc te)\n");
    printf("CSV: %s | Sai ket qua / loi: %d\n", csvPath.string().c_str(), failures);

    return failures ? 2 : 0;
}