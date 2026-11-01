#include "include/selection.hpp"

template <typename T>
SELECTION_SORT<T>::SELECTION_SORT(T size) {
    this->size = size;
    arr = new T[size];
}

template <typename T>
SELECTION_SORT<T>::~SELECTION_SORT() {
    delete[] arr;
}

template <typename T>
void SELECTION_SORT<T>::selection_sort_asc(T* arr, T size) {
    for (T i = 0; i < size - 1; i++) {
        T min_index = i;
        for (T j = i + 1; j < size; j++) {
            if (arr[j] < arr[min_index]) {
                min_index = j;
            }
        }
        swap(arr[i], arr[min_index]);
    }
}

template <typename T>
void SELECTION_SORT<T>::selection_sort_desc(T* arr, T size) {
    for (T i = 0; i < size - 1; i++) {
        T max_index = i;
        for (T j = i + 1; j < size; j++) {
            if (arr[j] > arr[max_index]) {
                max_index = j;
            }
        }
        swap(arr[i], arr[max_index]);
    }
}

template <typename T>
void SELECTION_SORT<T>::Sort(T* arr, T start, T end, bool ascending) {
    if (ascending) {
        selection_sort_asc(arr + start, end - start + 1);
    } else {
        selection_sort_desc(arr + start, end - start + 1);
    }
}