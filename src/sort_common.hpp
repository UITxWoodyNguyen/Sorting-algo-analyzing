// sort_common.hpp - phần dùng chung cho main.cpp và bench.cpp
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Các file *-sort.cpp dùng swap/min không có "std::" nên cần dòng này
// TRƯỚC khi include chúng.
using namespace std;
namespace fs = std::filesystem;

// Các thuật toán được cài dạng template nằm trong .cpp => phải #include thẳng .cpp
// (không biên dịch riêng các file này, nếu không sẽ lỗi linker "undefined reference").
#include "heap-sort.cpp"
#include "insertion-sort.cpp"
#include "merge-sort.cpp"
#include "quick-sort.cpp"
#include "selection-sort.cpp"

// T trong các lớp sort vừa là kiểu phần tử vừa là kiểu chỉ số (i >= 0, j >= 0, arr[left]...)
// => BẮT BUỘC là kiểu số nguyên CÓ DẤU (int / long long). Không dùng unsigned hay double.
using Int = long long;

constexpr bool ASCENDING = true;       // true: tăng dần, false: giảm dần
constexpr bool WRITE_N_HEADER = true;  // true: file .out có dòng đầu là n (giống .inp)
constexpr int NUM_ALGOS = 5;

inline const char* algoName(int a) {
    static const char* names[] = {"?", "Insertion", "Selection", "Heap", "Merge", "Quick"};
    return (a >= 1 && a <= NUM_ALGOS) ? names[a] : names[0];
}

// Insertion & Selection là O(n^2) -> bench sẽ bỏ qua khi n quá lớn
inline bool isQuadratic(int a) { return a == 1 || a == 2; }

template <class S>
inline void invokeSort(Int* a, Int n, bool asc) {
    S s(n);  // constructor có cấp phát mảng nội bộ, nhưng ta truyền mảng riêng vào Sort()
    s.Sort(a, 0, n - 1, asc);
}

// Sắp xếp v bằng thuật toán số algo (1..5)
inline void runSort(int algo, vector<Int>& v, bool asc = ASCENDING) {
    Int n = (Int)v.size();
    if (n <= 1) return;
    Int* a = v.data();
    switch (algo) {
        case 1: invokeSort<INSERTION_SORT<Int>>(a, n, asc); break;
        case 2: invokeSort<SELECTION_SORT<Int>>(a, n, asc); break;
        case 3: invokeSort<HEAP_SORT<Int>>(a, n, asc); break;
        case 4: invokeSort<MERGE_SORT<Int>>(a, n, asc); break;
        case 5: invokeSort<QUICK_SORT<Int>>(a, n, asc); break;
        default: throw invalid_argument("algo phai nam trong 1..5");
    }
}

// ---------- I/O ----------
// Định dạng .inp:  dòng 1: n ; tiếp theo: n số nguyên (cách nhau bằng khoảng trắng/xuống dòng)
inline bool readArray(istream& in, vector<Int>& a) {
    Int n;
    if (!(in >> n) || n < 0) return false;
    a.assign((size_t)n, 0);
    for (auto& x : a)
        if (!(in >> x)) return false;
    return true;
}

inline void writeArray(ostream& out, const vector<Int>& a, bool header = WRITE_N_HEADER) {
    if (header) out << a.size() << '\n';
    for (size_t i = 0; i < a.size(); i++) {
        if (i) out << ' ';
        out << a[i];
    }
    out << '\n';
}

// Đọc toàn bộ số trong file .out; nếu có dòng header n thì bỏ đi (để so sánh kết quả)
inline bool readExpected(const fs::path& p, size_t n, vector<Int>& t) {
    ifstream f(p);
    if (!f) return false;
    t.clear();
    Int x;
    while (f >> x) t.push_back(x);
    if (t.size() == n + 1 && t[0] == (Int)n) t.erase(t.begin());
    return true;
}

// Tên thư mục con theo thuật toán: dataset/output/<algoDir>/
inline const char* algoDir(int a) {
    static const char* dirs[] = {"unknown", "insertion", "selection", "heap", "merge", "quick"};
    return (a >= 1 && a <= NUM_ALGOS) ? dirs[a] : dirs[0];
}

// dataset/input/01.inp + heap sort -> dataset/output/heap/01.out
// (thay thành phần thư mục "input" gần nhất bằng "output/<algoDir>"; đổi đuôi .inp -> .out;
//  nếu đường dẫn không có thư mục "input" thì ghi vào <thư mục của file>/<algoDir>/)
inline fs::path toOutputPath(const fs::path& in, int algo) {
    fs::path dir = in.parent_path();  // giữ biến cục bộ, tránh begin()/end() của 2 temporary khác nhau
    vector<fs::path> parts(dir.begin(), dir.end());
    bool replaced = false;
    for (int i = (int)parts.size() - 1; i >= 0; --i) {
        if (parts[i] == "input") {
            parts[i] = "output";
            parts.insert(parts.begin() + i + 1, algoDir(algo));
            replaced = true;
            break;
        }
    }
    if (!replaced) parts.push_back(algoDir(algo));
    fs::path out;
    for (auto& p : parts) out /= p;
    out /= in.filename();
    out.replace_extension(".out");
    return out;
}