#include "app_modes.hpp"
#include "sort_common.hpp"

#include <cstdlib>

static void printUsage(const char* program) {
    cout << "Usage:\n"
         << "  " << program << "                         stdin/stdout mode\n"
         << "  " << program << " --file IN OUT ALGO       file input/output mode\n"
         << "  " << program << " --benchmark [options]     benchmark and CSV mode\n"
         << "  " << program << " --kat [options]           known-answer test mode\n\n"
         << "ALGO: 1=insertion, 2=selection, 3=heap, 4=merge, 5=quick\n\n"
         << "Benchmark options:\n"
         << "  --in DIR       input directory (default: dataset/input)\n"
         << "  --out DIR      expected output directory (default: dataset/output)\n"
         << "  --csv FILE     CSV output (default: Output/benchmark.csv)\n"
         << "  --repeat N     repetitions (default: 3)\n"
         << "  --timeout SEC  time limit per run (default: 180)\n"
         << "  --skip-after-tle\n\n"
         << "KAT options:\n"
         << "  --vectors FILE --algo NAME|1..5 --category NAME --timeout SEC -v\n";
}

static bool parseAlgo(const char* text, int& algo) {
    char* end = nullptr;
    long value = strtol(text, &end, 10);
    if (*text && *end == '\0' && value >= 1 && value <= NUM_ALGOS) {
        algo = static_cast<int>(value);
        return true;
    }
    for (int candidate = 1; candidate <= NUM_ALGOS; ++candidate) {
        if (string(text) == algoDir(candidate)) {
            algo = candidate;
            return true;
        }
    }
    return false;
}

static int stdinMode(int algo) {
    if (algo == 0) {
        cout << "Chon thuat toan:\n"
             << "  1. Insertion sort\n  2. Selection sort\n  3. Heap sort\n"
             << "  4. Merge sort\n  5. Quick sort\nLua chon (1-5): " << flush;
        if (!(cin >> algo) || algo < 1 || algo > NUM_ALGOS) {
            cerr << "Loi: thuat toan phai nam trong 1..5\n";
            return 1;
        }
    }

    vector<Int> values;
    if (!readArray(cin, values)) {
        cerr << "Loi: stdin phai co n va n so nguyen\n";
        return 1;
    }
    runSort(algo, values);
    writeArray(cout, values);
    return 0;
}

static int fileMode(const fs::path& input, const fs::path& output, int algo) {
    if (algo < 1 || algo > NUM_ALGOS) {
        cerr << "Loi: thuat toan phai nam trong 1..5\n";
        return 1;
    }
    ifstream inputFile(input);
    if (!inputFile) {
        cerr << "Loi: khong mo duoc file " << input << '\n';
        return 1;
    }
    vector<Int> values;
    if (!readArray(inputFile, values)) {
        cerr << "Loi: file input khong hop le " << input << '\n';
        return 1;
    }
    runSort(algo, values);
    if (output.has_parent_path()) fs::create_directories(output.parent_path());
    ofstream outputFile(output);
    if (!outputFile) {
        cerr << "Loi: khong ghi duoc file " << output << '\n';
        return 1;
    }
    writeArray(outputFile, values);
    cout << "Da ghi " << output << " (" << algoName(algo) << ", n=" << values.size() << ")\n";
    return 0;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc == 1) return stdinMode(0);
    string mode = argv[1];
    if (mode == "--help" || mode == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    if (mode == "--benchmark" || mode == "benchmark")
        return runBenchmark(argc - 1, argv + 1);
    if (mode == "--kat" || mode == "kat")
        return runKnownAnswerTests(argc - 1, argv + 1);
    if (mode == "--file" || mode == "file") {
        if (argc != 5) {
            printUsage(argv[0]);
            return 1;
        }
        int algo = 0;
        if (!parseAlgo(argv[4], algo)) {
            cerr << "Loi: algorithm khong hop le: " << argv[4] << '\n';
            return 1;
        }
        return fileMode(argv[2], argv[3], algo);
    }
    printUsage(argv[0]);
    return 1;
}
