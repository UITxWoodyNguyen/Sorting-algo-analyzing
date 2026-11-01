#include "include/heap.hpp"

template <typename T>
HEAP_SORT<T>::HEAP_SORT(T size) {
    this->size = size;
    arr = new T[size];
}

template <typename T>
HEAP_SORT<T>::~HEAP_SORT() {
    delete[] arr;
}

template <typename T>
void HEAP_SORT<T>::heapify_asc(T* arr, T size, T i) {
    T largest = i;
    T left = 2 * i + 1;
    T right = 2 * i + 2;

    if (left < size && arr[left] > arr[largest]) largest = left;
    if (right < size && arr[right] > arr[largest]) largest = right;

    if (largest != i) {
        swap(arr[i], arr[largest]);
        heapify_asc(arr, size, largest);
    }
}

template <typename T>
void HEAP_SORT<T>::heapify_desc(T* arr, T size, T i) {
    T smallest = i;
    T left = 2 * i + 1;
    T right = 2 * i + 2;

    if (left < size && arr[left] < arr[smallest]) smallest = left;
    if (right < size && arr[right] < arr[smallest]) smallest = right;

    if (smallest != i) {
        swap(arr[i], arr[smallest]);
        heapify_desc(arr, size, smallest);
    }
}

template <typename T>
void HEAP_SORT<T>::buildMaxHeap(T* arr, T size) {
    for (T i = size / 2 - 1; i >= 0; i--) 
        heapify_asc(arr, size, i);
    
}

template <typename T>
void HEAP_SORT<T>::buildMinHeap(T* arr, T size) {
    for (T i = size / 2 - 1; i >= 0; i--) 
        heapify_desc(arr, size, i);
    
}

template <typename T>
void HEAP_SORT<T>::heap_sort_asc(T* arr, T size) {
    buildMaxHeap(arr, size);
    for (T i = size - 1; i >= 0; i--) {
        swap(arr[0], arr[i]);
        heapify_asc(arr, i, 0);
    }
}

template <typename T>
void HEAP_SORT<T>::heap_sort_desc(T* arr, T size) {
    buildMinHeap(arr, size);
    for (T i = size - 1; i >= 0; i--) {
        swap(arr[0], arr[i]);
        heapify_desc(arr, i, 0);
    }
}   

template <typename T>
void HEAP_SORT<T>::Sort(T* arr, T start, T end, bool ascending) {
    if (ascending) heap_sort_asc(arr + start, end - start + 1);
    else heap_sort_desc(arr + start, end - start + 1);
}