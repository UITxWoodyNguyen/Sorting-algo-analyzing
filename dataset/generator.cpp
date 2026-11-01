#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <stdexcept>

namespace fs = std::filesystem;

// Default sizes: {100, 1e3, [1, 8]e4, [1, 8]e5, 1e6}
const std::vector<int> DEFAULT_SIZES = {
    100,             // 01: 100
    1000,            // 02: 1e3
    10000,           // 03: 1e4
    20000,           // 04: 2e4
    30000,           // 05: 3e4
    40000,           // 06: 4e4
    50000,           // 07: 5e4
    60000,           // 08: 6e4
    70000,           // 09: 7e4
    80000,           // 10: 8e4
    100000,          // 11: 1e5
    200000,          // 12: 2e5
    300000,          // 13: 3e5
    400000,          // 14: 4e5
    500000,          // 15: 5e5
    600000,          // 16: 6e5
    700000,          // 17: 7e5
    800000,          // 18: 8e5
    1000000          // 19: 1e6
};

const int TOTAL_TESTS = 50;
const std::vector<std::string> SORT_ALGOS = {
    "heap",
    "insertion",
    "merge",
    "quick",
    "selection"
};

// Determine base directory relative to working directory
fs::path getDatasetPath() {
    if (fs::exists("dataset")) {
        return "dataset";
    }
    if (fs::exists("input") && fs::exists("output")) {
        return ".";
    }
    return "dataset";
}

// Generate all 50 sizes: 19 default + 31 random sizes
std::vector<int> generateAllSizes(std::mt19937_64& rng) {
    std::vector<int> sizes = DEFAULT_SIZES; // 19 default sizes

    // Generate 31 random sizes across different scales up to 1,000,000
    // Include varied distributions (small, medium, large)
    std::uniform_int_distribution<int> distSmall(100, 10000);
    std::uniform_int_distribution<int> distMed(10000, 100000);
    std::uniform_int_distribution<int> distLarge(100000, 1000000);

    std::vector<int> randomSizes;
    for (int i = 0; i < 5; i++) {
        randomSizes.push_back(distSmall(rng));
    }
    for (int i = 0; i < 10; i++) {
        randomSizes.push_back(distMed(rng));
    }
    for (int i = 0; i < 16; i++) {
        randomSizes.push_back(distLarge(rng));
    }

    std::sort(randomSizes.begin(), randomSizes.end());
    sizes.insert(sizes.end(), randomSizes.begin(), randomSizes.end());

    if (sizes.size() != TOTAL_TESTS) {
        throw std::logic_error("The generated size list must contain exactly 50 test cases.");
    }

    return sizes;
}

// Generate test input files [01->50].inp
void generateInputs(const fs::path& datasetDir, const std::vector<int>& sizes, std::mt19937_64& rng) {
    fs::path inputDir = datasetDir / "input";
    fs::create_directories(inputDir);

    std::uniform_int_distribution<int> valDist(-1000000, 1000000);

    std::cout << "========================================================\n";
    std::cout << "           GENERATING 50 TEST INPUT FILES               \n";
    std::cout << "========================================================\n";
    std::cout << "Target Directory: " << inputDir.string() << "\n\n";

    // 1MB buffer for fast file output
    std::vector<char> buffer(1024 * 1024);

    for (int idx = 1; idx <= TOTAL_TESTS; ++idx) {
        int n = sizes[idx - 1];

        std::ostringstream filenameStream;
        filenameStream << std::setw(2) << std::setfill('0') << idx << ".inp";
        fs::path filePath = inputDir / filenameStream.str();

        std::ofstream fout(filePath, std::ios::out | std::ios::trunc);
        if (!fout.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }

        fout.rdbuf()->pubsetbuf(buffer.data(), buffer.size());

        // First line: array size N
        fout << n << "\n";

        // Second line: N space-separated random integers
        for (int i = 0; i < n; ++i) {
            fout << valDist(rng) << (i + 1 == n ? "" : " ");
        }
        fout << "\n";
        fout.close();

        std::string type = (idx <= 19) ? "[Default]" : "[Random ]";
        std::cout << "Generated " << filenameStream.str() << " " 
                  << type << " - Size: " << std::setw(8) << n << " elements\n";
    }

    std::cout << "\n>>> Successfully created all " << TOTAL_TESTS << " input files in " << inputDir.string() << " <<<\n\n";
}

// Generate sorted output files [01->50].out for each algorithm
void generateOutputs(const fs::path& datasetDir) {
    fs::path inputDir = datasetDir / "input";
    fs::path outputDir = datasetDir / "output";

    for (const auto& algo : SORT_ALGOS) {
        fs::create_directories(outputDir / algo);
    }

    std::cout << "========================================================\n";
    std::cout << "           GENERATING GROUND-TRUTH OUTPUTS              \n";
    std::cout << "========================================================\n";

    std::vector<char> inBuffer(1024 * 1024);
    std::vector<char> outBuffer(1024 * 1024);

    for (int idx = 1; idx <= TOTAL_TESTS; ++idx) {
        std::ostringstream baseNameStream;
        baseNameStream << std::setw(2) << std::setfill('0') << idx;
        std::string baseName = baseNameStream.str();

        fs::path inPath = inputDir / (baseName + ".inp");
        if (!fs::exists(inPath)) {
            std::cerr << "Input file not found: " << inPath << "\n";
            continue;
        }

        std::ifstream fin(inPath);
        if (!fin.is_open()) {
            throw std::runtime_error("Failed to open file: " + inPath.string());
        }
        fin.rdbuf()->pubsetbuf(inBuffer.data(), inBuffer.size());
        int n;
        if (!(fin >> n) || n < 0) {
            throw std::runtime_error("Invalid array size in file: " + inPath.string());
        }

        std::vector<int> arr(n);
        for (int i = 0; i < n; ++i) {
            if (!(fin >> arr[i])) {
                throw std::runtime_error("Invalid array data in file: " + inPath.string());
            }
        }
        fin.close();

        // Sort ascending
        std::sort(arr.begin(), arr.end());

        // Write to all algorithm output folders
        for (const auto& algo : SORT_ALGOS) {
            fs::path outPath = outputDir / algo / (baseName + ".out");
            std::ofstream fout(outPath, std::ios::out | std::ios::trunc);
            if (!fout.is_open()) {
                throw std::runtime_error("Failed to open file: " + outPath.string());
            }
            fout.rdbuf()->pubsetbuf(outBuffer.data(), outBuffer.size());

            for (int i = 0; i < n; ++i) {
                fout << arr[i] << (i + 1 == n ? "" : " ");
            }
            fout << "\n";
            fout.close();
        }

        std::cout << "Generated output for test " << baseName << " (" << n << " elements)\n";
    }

    std::cout << "\n>>> Output files generated for all algorithms <<<\n\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    fs::path datasetDir = getDatasetPath();

    // Use a fixed seed for reproducibility across benchmarks
    uint64_t seed = 20261101ULL;
    std::mt19937_64 rng(seed);

    bool genInp = true;
    bool genOut = false;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  (no args)       Generate 50 input test cases (01->50.inp)\n";
            std::cout << "  --inp, -i       Generate 50 input test cases\n";
            std::cout << "  --out, -o       Generate output files for all algorithms\n";
            std::cout << "  --all, -a       Generate both input and output files\n";
            return 0;
        } else if (arg == "--out" || arg == "-o") {
            genInp = false;
            genOut = true;
        } else if (arg == "--all" || arg == "-a") {
            genInp = true;
            genOut = true;
        } else if (arg == "--inp" || arg == "-i") {
            genInp = true;
            genOut = false;
        }
    }

    try {
        std::vector<int> sizes = generateAllSizes(rng);

        if (genInp) {
            generateInputs(datasetDir, sizes, rng);
        }

        if (genOut) {
            generateOutputs(datasetDir);
        }
    } catch (const std::exception& error) {
        std::cerr << "Generation failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
