#include "include/merge.hpp"

template <typename T>
MERGE_SORT<T>::MERGE_SORT(T size) {
    this->size = size;
    arr = new T[size];
}

template <typename T>
MERGE_SORT<T>::~MERGE_SORT() {
    delete[] arr;
}

template <typename T>
void MERGE_SORT<T>::merge(T* arr, T start, T mid, T end, T* temp, bool ascending) {
    T i = start;
    T j = mid + 1;
    T k = start;

    while (i <= mid && j <= end) {
        if ((ascending && arr[i] <= arr[j]) || (!ascending && arr[i] >= arr[j])) 
            temp[k++] = arr[i++];
        else 
            temp[k++] = arr[j++];
    }

    while (i <= mid) temp[k++] = arr[i++];
    while (j <= end) temp[k++] = arr[j++];

    for (T l = start; l <= end; l++) arr[l] = temp[l];
}

template <typename T>
void MERGE_SORT<T>::merge_sort_asc(T* arr, T start, T end) {
    if (start > end) return;

    T* temp = new T[end - start + 1];
    for (T siz = 1; siz <= end - start; siz <<= 1) {
        for (T left = start; left < end; left += (siz << 1)) {
            T mid = min(left + siz - 1, end);
            T right = min(left + 2 * siz - 1, end);
            merge(arr, left, mid, right, temp, true);
        }
    }
    delete[] temp;
}

template <typename T>
void MERGE_SORT<T>::merge_sort_desc(T* arr, T start, T end) {
    if (start > end) return;

    T* temp = new T[end - start + 1];
    for (T siz = 1; siz <= end - start; siz <<= 1) {
        for (T left = start; left < end; left += (siz << 1)) {
            T mid = min(left + siz - 1, end);
            T right = min(left + 2 * siz - 1, end);
            merge(arr, left, mid, right, temp, false);
        }
    }
    delete[] temp;
}

template <typename T>
void MERGE_SORT<T>::Sort(T* arr, T start, T end, bool ascending) {
    if (ascending) merge_sort_asc(arr, start, end);
    else merge_sort_desc(arr, start, end);
}