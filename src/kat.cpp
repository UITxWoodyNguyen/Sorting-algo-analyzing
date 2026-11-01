// kat.cpp - Known Answer Test cho 5 thuật toán sort
//
//   ./kat [--vectors test-vector/vectors.json] [--algo heap|3] [--category edge]
//         [--timeout 10] [-v]
//
//   --algo       chỉ test 1 thuật toán (tên: insertion|selection|heap|merge|quick hoặc số 1..5)
//   --category   chỉ test 1 nhóm: best | worst | normal | edge
//   --timeout    time limit (giây) cho mỗi lần chạy 1 case (mặc định 10)
//   -v           in cả các test PASS (mặc định chỉ in test lỗi)
//
// Mỗi case trong vectors.json được chạy với từng thuật toán (hoặc chỉ các thuật toán trong "algos"),
// theo cả 2 chiều tăng dần (expected_asc) và giảm dần (expected_desc).
// Trên Linux/macOS mỗi test chạy trong tiến trình con => crash / treo vòng lặp vô hạn
// chỉ làm test đó bị CRASH / TIMEOUT, không làm sập cả KAT.
#include "sort_common.hpp"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#if !defined(_WIN32) && !defined(KAT_NO_FORK)
#define KAT_USE_FORK 1
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

// =====================================================================
//  JSON parser tối giản (đủ cho vectors.json: object / array / string / số nguyên / bool / null)
// =====================================================================
struct Json {
    enum Type { Null, Bool, Num, Str, Arr, Obj } type = Null;
    bool b = false;
    long long num = 0;
    string str;
    vector<Json> arr;
    vector<pair<string, Json>> obj;

    const Json* find(const string& k) const {
        for (auto& kv : obj)
            if (kv.first == k) return &kv.second;
        return nullptr;
    }
};

class JsonParser {
    const string& s;
    size_t p = 0;

    [[noreturn]] void fail(const string& msg) const {
        throw runtime_error("JSON loi tai vi tri " + to_string(p) + ": " + msg);
    }
    void ws() {
        while (p < s.size() && isspace((unsigned char)s[p])) p++;
    }
    bool eat(char c) {
        ws();
        if (p < s.size() && s[p] == c) {
            p++;
            return true;
        }
        return false;
    }
    void expect(char c) {
        if (!eat(c)) fail(string("can ky tu '") + c + "'");
    }
    bool literal(const char* w) {
        size_t n = strlen(w);
        if (s.compare(p, n, w) == 0) {
            p += n;
            return true;
        }
        return false;
    }

    string string_() {
        expect('"');
        string r;
        while (p < s.size() && s[p] != '"') {
            char c = s[p++];
            if (c != '\\') {
                r += c;
                continue;
            }
            if (p >= s.size()) fail("escape bi cat");
            char e = s[p++];
            switch (e) {
                case '"': r += '"'; break;
                case '\\': r += '\\'; break;
                case '/': r += '/'; break;
                case 'n': r += '\n'; break;
                case 't': r += '\t'; break;
                case 'r': r += '\r'; break;
                case 'b': r += '\b'; break;
                case 'f': r += '\f'; break;
                case 'u': r += '?'; p += 4; break;  // không cần dùng tới trong vectors
                default: fail("escape khong hop le");
            }
        }
        if (p >= s.size()) fail("chuoi khong dong");
        p++;  // dấu " đóng
        return r;
    }

    Json number_() {
        size_t st = p;
        if (p < s.size() && s[p] == '-') p++;
        while (p < s.size() && isdigit((unsigned char)s[p])) p++;
        if (p < s.size() && (s[p] == '.' || s[p] == 'e' || s[p] == 'E')) fail("chi ho tro so nguyen");
        if (p == st || (p == st + 1 && s[st] == '-')) fail("so khong hop le");
        Json j;
        j.type = Json::Num;
        errno = 0;
        j.num = strtoll(s.substr(st, p - st).c_str(), nullptr, 10);
        if (errno == ERANGE) fail("so ngoai pham vi 64-bit");
        return j;
    }

    Json value() {
        ws();
        if (p >= s.size()) fail("het du lieu");
        Json j;
        char c = s[p];
        if (c == '{') {
            p++;
            j.type = Json::Obj;
            if (!eat('}')) {
                do {
                    ws();
                    string k = string_();
                    expect(':');
                    j.obj.emplace_back(move(k), value());
                } while (eat(','));
                expect('}');
            }
        } else if (c == '[') {
            p++;
            j.type = Json::Arr;
            if (!eat(']')) {
                do j.arr.push_back(value());
                while (eat(','));
                expect(']');
            }
        } else if (c == '"') {
            j.type = Json::Str;
            j.str = string_();
        } else if (literal("true")) {
            j.type = Json::Bool;
            j.b = true;
        } else if (literal("false")) {
            j.type = Json::Bool;
        } else if (literal("null")) {
            j.type = Json::Null;
        } else {
            j = number_();
        }
        return j;
    }

public:
    explicit JsonParser(const string& src) : s(src) {}
    Json parse() {
        Json j = value();
        ws();
        if (p != s.size()) fail("du lieu thua sau JSON");
        return j;
    }
};

// =====================================================================
//  Nạp test case
// =====================================================================
static const char* CATEGORIES[] = {"best", "worst", "normal", "edge"};
constexpr int NUM_CATS = 4;

static int categoryIndex(const string& c) {
    for (int i = 0; i < NUM_CATS; i++)
        if (c == CATEGORIES[i]) return i;
    return -1;
}

static int algoByName(const string& s) {  // "heap" hoặc "3" -> 3 ; không hợp lệ -> 0
    for (int a = 1; a <= NUM_ALGOS; a++)
        if (s == algoDir(a) || s == to_string(a)) return a;
    return 0;
}

struct Case {
    string id, category, name, note;
    int cat = -1;
    vector<int> algos;
    vector<Int> in, expAsc, expDesc;
};

static vector<Int> toVector(const Json* j, const string& field, const string& id) {
    if (!j || j->type != Json::Arr) throw runtime_error("case " + id + ": thieu mang \"" + field + "\"");
    vector<Int> v;
    v.reserve(j->arr.size());
    for (auto& e : j->arr) {
        if (e.type != Json::Num) throw runtime_error("case " + id + ": \"" + field + "\" chua phan tu khong phai so");
        v.push_back((Int)e.num);
    }
    return v;
}

static string strField(const Json& c, const char* k, bool required, const string& id) {
    const Json* j = c.find(k);
    if (!j || j->type != Json::Str) {
        if (required) throw runtime_error("case " + id + ": thieu truong \"" + k + "\"");
        return "";
    }
    return j->str;
}

static vector<Case> loadCases(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) throw runtime_error("khong mo duoc file " + path);
    string text((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    Json root = JsonParser(text).parse();

    const Json* cs = root.find("cases");
    if (!cs || cs->type != Json::Arr) throw runtime_error("vectors.json thieu mang \"cases\"");

    vector<Case> cases;
    for (size_t idx = 0; idx < cs->arr.size(); idx++) {
        const Json& c = cs->arr[idx];
        Case k;
        k.id = strField(c, "id", false, "#" + to_string(idx));
        if (k.id.empty()) k.id = "#" + to_string(idx);
        k.category = strField(c, "category", true, k.id);
        k.name = strField(c, "name", true, k.id);
        k.note = strField(c, "note", false, k.id);
        k.cat = categoryIndex(k.category);
        if (k.cat < 0) throw runtime_error("case " + k.id + ": category khong hop le: " + k.category);

        k.in = toVector(c.find("input"), "input", k.id);
        k.expAsc = toVector(c.find("expected_asc"), "expected_asc", k.id);
        k.expDesc = toVector(c.find("expected_desc"), "expected_desc", k.id);

        const Json* al = c.find("algos");
        if (al) {
            if (al->type != Json::Arr) throw runtime_error("case " + k.id + ": \"algos\" phai la mang");
            for (auto& e : al->arr) {
                int a = e.type == Json::Str ? algoByName(e.str) : 0;
                if (!a) throw runtime_error("case " + k.id + ": ten thuat toan khong hop le trong \"algos\"");
                k.algos.push_back(a);
            }
        } else {
            for (int a = 1; a <= NUM_ALGOS; a++) k.algos.push_back(a);
        }
        cases.push_back(move(k));
    }
    return cases;
}

// =====================================================================
//  Chạy 1 test
// =====================================================================
enum class Verdict { PASS, FAIL, CRASH, TIMEOUT };

static string preview(const vector<Int>& v) {
    string r = "[";
    for (size_t i = 0; i < v.size() && i < 12; i++) {
        if (i) r += ", ";
        r += to_string(v[i]);
    }
    if (v.size() > 12) r += ", ...";
    return r + "] (n=" + to_string(v.size()) + ")";
}

static string describeDiff(const vector<Int>& got, const vector<Int>& exp) {
    string d;
    if (got.size() != exp.size()) {
        d = "sai kich thuoc: got " + to_string(got.size()) + ", expected " + to_string(exp.size());
    } else {
        for (size_t i = 0; i < got.size(); i++)
            if (got[i] != exp[i]) {
                d = "khac nhau tai index " + to_string(i) + ": got " + to_string(got[i]) + ", expected " +
                    to_string(exp[i]);
                break;
            }
    }
    return d + " | got=" + preview(got);
}

#ifdef KAT_USE_FORK
static Verdict runOne(int algo, const Case& c, bool asc, double timeoutSec, string& detail) {
    const vector<Int>& expected = asc ? c.expAsc : c.expDesc;
    int fd[2];
    if (pipe(fd) != 0) {
        detail = "pipe() that bai";
        return Verdict::CRASH;
    }
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid < 0) {
        close(fd[0]);
        close(fd[1]);
        detail = "fork() that bai";
        return Verdict::CRASH;
    }
    if (pid == 0) {  // ----- tiến trình con -----
        close(fd[0]);
        vector<Int> v = c.in;
        runSort(algo, v, asc);
        if (v == expected) _exit(0);
        string d = describeDiff(v, expected);
        if (d.size() > 900) d.resize(900);
        ssize_t w = write(fd[1], d.data(), d.size());
        (void)w;
        _exit(1);
    }

    // ----- tiến trình cha -----
    close(fd[1]);
    auto t0 = chrono::steady_clock::now();
    int status = 0;
    Verdict vd;
    while (true) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) break;
        if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() > timeoutSec) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            close(fd[0]);
            detail = "vuot qua " + to_string(timeoutSec) + " s (co the vong lap vo han)";
            return Verdict::TIMEOUT;
        }
        usleep(200);
    }

    char buf[1024];
    ssize_t n = read(fd[0], buf, sizeof buf - 1);  // con đã thoát nên không bị chặn
    close(fd[0]);
    if (n > 0) detail.assign(buf, (size_t)n);

    if (WIFSIGNALED(status)) {
        detail = string("bi giet boi tin hieu ") + strsignal(WTERMSIG(status));
        vd = Verdict::CRASH;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        vd = Verdict::PASS;
    } else {
        vd = Verdict::FAIL;
    }
    return vd;
}
#else
// Windows / -DKAT_NO_FORK: chạy ngay trong tiến trình hiện tại (không có cách ly crash/timeout)
static Verdict runOne(int algo, const Case& c, bool asc, double, string& detail) {
    vector<Int> v = c.in;
    const vector<Int>& expected = asc ? c.expAsc : c.expDesc;
    try {
        runSort(algo, v, asc);
    } catch (const exception& e) {
        detail = string("exception: ") + e.what();
        return Verdict::CRASH;
    }
    if (v == expected) return Verdict::PASS;
    detail = describeDiff(v, expected);
    return Verdict::FAIL;
}
#endif

// =====================================================================
int runKnownAnswerTests(int argc, char** argv) {
    string vectorsPath = "test-vector/vectors.json";
    int onlyAlgo = 0, onlyCat = -1;
    double timeoutSec = 10.0;
    bool verbose = false;

    for (int i = 1; i < argc; i++) {
        string k = argv[i];
        if (k == "-v") {
            verbose = true;
            continue;
        }
        if (i + 1 >= argc) {
            cerr << "Thieu gia tri cho " << k << '\n';
            return 1;
        }
        string v = argv[++i];
        if (k == "--vectors") vectorsPath = v;
        else if (k == "--timeout") timeoutSec = max(0.01, atof(v.c_str()));
        else if (k == "--algo") {
            onlyAlgo = algoByName(v);
            if (!onlyAlgo) {
                cerr << "Thuat toan khong hop le: " << v << '\n';
                return 1;
            }
        } else if (k == "--category") {
            onlyCat = categoryIndex(v);
            if (onlyCat < 0) {
                cerr << "Category khong hop le: " << v << " (best|worst|normal|edge)\n";
                return 1;
            }
        } else {
            cerr << "Tham so khong hop le: " << k << '\n';
            return 1;
        }
    }

    vector<Case> cases;
    try {
        cases = loadCases(vectorsPath);
    } catch (const exception& e) {
        cerr << "Loi nap vectors: " << e.what() << '\n';
        return 1;
    }

    int pass[NUM_ALGOS + 1][NUM_CATS] = {}, total[NUM_ALGOS + 1][NUM_CATS] = {};
    int nFail = 0, nCrash = 0, nTimeout = 0;

    printf("KAT: %zu case tu %s | timeout %g s/test\n\n", cases.size(), vectorsPath.c_str(), timeoutSec);

    for (auto& c : cases) {
        if (onlyCat >= 0 && c.cat != onlyCat) continue;
        for (int algo : c.algos) {
            if (onlyAlgo && algo != onlyAlgo) continue;
            for (int dir = 0; dir < 2; dir++) {
                bool asc = (dir == 0);
                string detail;
                Verdict vd = runOne(algo, c, asc, timeoutSec, detail);

                total[algo][c.cat]++;
                if (vd == Verdict::PASS) pass[algo][c.cat]++;
                else if (vd == Verdict::FAIL) nFail++;
                else if (vd == Verdict::CRASH) nCrash++;
                else nTimeout++;

                if (vd != Verdict::PASS || verbose) {
                    const char* tag = vd == Verdict::PASS ? "PASS" : vd == Verdict::FAIL ? "FAIL"
                                    : vd == Verdict::CRASH ? "CRASH" : "TIMEOUT";
                    printf("[%-7s] %-9s %-9s %-34s %s\n", tag, algoDir(algo), c.id.c_str(), c.name.c_str(),
                           asc ? "asc" : "desc");
                    if (vd != Verdict::PASS && !detail.empty()) printf("           -> %s\n", detail.c_str());
                    fflush(stdout);
                }
            }
        }
    }

    // ----- Bảng tổng kết: thuật toán x nhóm (pass/total) -----
    printf("\n%-11s", "Algorithm");
    for (int k = 0; k < NUM_CATS; k++) printf(" %9s", CATEGORIES[k]);
    printf(" %10s\n", "TOTAL");

    int grandPass = 0, grandTotal = 0;
    for (int a = 1; a <= NUM_ALGOS; a++) {
        int rowP = 0, rowT = 0;
        for (int k = 0; k < NUM_CATS; k++) {
            rowP += pass[a][k];
            rowT += total[a][k];
        }
        if (rowT == 0) continue;
        printf("%-11s", algoDir(a));
        for (int k = 0; k < NUM_CATS; k++) {
            char cell[24];
            if (total[a][k] == 0) snprintf(cell, sizeof cell, "-");
            else snprintf(cell, sizeof cell, "%d/%d", pass[a][k], total[a][k]);
            printf(" %9s", cell);
        }
        printf(" %10s%s\n", (to_string(rowP) + "/" + to_string(rowT)).c_str(), rowP == rowT ? "" : "  <-- LOI");
        grandPass += rowP;
        grandTotal += rowT;
    }

    printf("\nTong: %d/%d pass | FAIL %d | CRASH %d | TIMEOUT %d\n", grandPass, grandTotal, nFail, nCrash,
           nTimeout);

    if (grandTotal == 0) {
        printf("Khong co test nao duoc chay (kiem tra lai --algo / --category).\n");
        return 2;
    }
    return grandPass == grandTotal ? 0 : 1;
}