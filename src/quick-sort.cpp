#include "include/quick.hpp"

template <typename T>
QUICK_SORT<T>::QUICK_SORT(T size) {
    this->size = size;
    arr = new T[size];
}

template <typename T>
QUICK_SORT<T>::~QUICK_SORT() {
    delete[] arr;
}

template <typename T>
void QUICK_SORT<T>::partition_asc(T* arr, T, T, T pivot, T& i, T& j) {
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }
}

template <typename T>
void QUICK_SORT<T>::partition_desc(T* arr, T, T, T pivot, T& i, T& j) {
    while (i <= j) {
        while (arr[i] > pivot) i++;
        while (arr[j] < pivot) j--;
        if (i <= j) {
            swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }
}

template <typename T>
void QUICK_SORT<T>::quick_sort_asc(T* arr, T start, T end) {
    if (start >= end) return;

    T pivot = arr[start + (end - start) / 2];
    T i = start;
    T j = end;

    partition_asc(arr, start, end, pivot, i, j);

    if (start < j) quick_sort_asc(arr, start, j);
    if (i < end) quick_sort_asc(arr, i, end);
}

template <typename T>
void QUICK_SORT<T>::quick_sort_desc(T* arr, T start, T end) {
    if (start >= end) return;

    T pivot = arr[start + (end - start) / 2];
    T i = start;
    T j = end;

    partition_desc(arr, start, end, pivot, i, j);

    if (start < j) quick_sort_desc(arr, start, j);
    if (i < end) quick_sort_desc(arr, i, end);
}

template <typename T>
void QUICK_SORT<T>::Sort(T* arr, T start, T end, bool ascending) {
    if (ascending) quick_sort_asc(arr, start, end);
    else quick_sort_desc(arr, start, end);
}