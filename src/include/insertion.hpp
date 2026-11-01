#pragma once

#include <iostream>
#include <string>
using namespace std;

template <typename T>
class INSERTION_SORT {
    private:
        int size;
        T* arr;

    public:
        INSERTION_SORT(T size);
        ~INSERTION_SORT();
        void insertion_sort_asc(T* arr, T size);
        void insertion_sort_desc(T* arr, T size);
        void Sort(T* arr, T start, T end, bool ascending = true);
};