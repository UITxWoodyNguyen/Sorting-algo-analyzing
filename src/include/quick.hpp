#pragma once

#include <iostream>
#include <string>
using namespace std;

template <typename T>
class QUICK_SORT {
    private:
        int size;
        T* arr;

    public:
        QUICK_SORT(T size);
        ~QUICK_SORT();
        void partition_asc(T* arr, T start, T end, T pivot, T& i, T& j);
        void partition_desc(T* arr, T start, T end, T pivot, T& i, T& j);
        void quick_sort_asc(T* arr, T start, T end);
        void quick_sort_desc(T* arr, T start, T end);
        void Sort(T* arr, T start, T end, bool ascending = true);
};