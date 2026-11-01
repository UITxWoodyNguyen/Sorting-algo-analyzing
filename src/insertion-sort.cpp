#include "include/insertion.hpp"

template <typename T>
INSERTION_SORT<T>::INSERTION_SORT(T size) {
    this->size = size;
    arr = new T[size];
}

template <typename T>
INSERTION_SORT<T>::~INSERTION_SORT() {
    delete[] arr;
}

template <typename T>
void INSERTION_SORT<T>::insertion_sort_asc(T* arr, T size) {
    for (T i = 1; i < size; i++) {
        T key = arr[i];
        T j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

template <typename T>
void INSERTION_SORT<T>::insertion_sort_desc(T* arr, T size) {
    for (T i = 1; i < size; i++) {
        T key = arr[i];
        T j = i - 1;
        while (j >= 0 && arr[j] < key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

template <typename T>
void INSERTION_SORT<T>::Sort(T* arr, T start, T end, bool ascending) {
    if (ascending) insertion_sort_asc(arr + start, end - start + 1);
    else insertion_sort_desc(arr + start, end - start + 1);
}