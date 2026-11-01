#pragma once

#include <iostream>
#include <string>

using namespace std;

template <typename T>
class MERGE_SORT {
    private:
        int size;
        T* arr;

    public:
        MERGE_SORT(T size);
        ~MERGE_SORT();
        void merge(T* arr, T start, T mid, T end, T* temp, bool ascending);
        void merge_sort_asc(T* arr, T start, T end);
        void merge_sort_desc(T* arr, T start, T end);
        void Sort(T* arr, T start, T end, bool ascending = true);
};