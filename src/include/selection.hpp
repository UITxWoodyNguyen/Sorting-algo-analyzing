#pragma once

#include <iostream>
#include <string>

using namespace std;

template <typename T>
class SELECTION_SORT {
    private:
        int size;
        T* arr;

    public:
        SELECTION_SORT(T size);
        ~SELECTION_SORT();
        void selection_sort_asc(T* arr, T size);
        void selection_sort_desc(T* arr, T size);
        void Sort(T* arr, T start, T end, bool ascending = true);
};