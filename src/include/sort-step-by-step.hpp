#pragma once

#include <iostream>
#include <vector>
#include <string>
using namespace std;

class SHOW_SORT_STEP {
    private:
        int size;
        vector<int> arr;
    public:
        SHOW_SORT_STEP(int size, vector<int> arr);
        static void print_array(vector<int> arr);

        void selection_sort(vector<int> arr);
        void insertion_sort(vector<int> arr);

        void partition(vector<int> arr, int low, int high, int &i, int &j, int pivot_index);
        void quick_sort(vector<int> arr, int low, int high);

        void merge(vector<int> arr, int left, int mid, int right);
        void merge_sort(vector<int> arr, int left, int right);

        void heapify(vector<int> arr, int n, int i);
        void build_heap(vector<int> arr, int n);
        void heap_sort(vector<int> arr);
};