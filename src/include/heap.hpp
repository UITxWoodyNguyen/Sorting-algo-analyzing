#pragma once

#include <iostream>
#include <string>
using namespace std;

template <typename T>
class HEAP_SORT {
    private:
        int size;
        T* arr;

    public:
        HEAP_SORT(T size);
        ~HEAP_SORT();
        void heapify_asc(T* arr, T size, T i);
        void heapify_desc(T* arr, T size, T i);
        void buildMaxHeap(T* arr, T size);
        void buildMinHeap(T* arr, T size);
        void heap_sort_asc(T* arr, T size);
        void heap_sort_desc(T* arr, T size);
        void Sort(T* arr, T start, T end, bool ascending = true);
};