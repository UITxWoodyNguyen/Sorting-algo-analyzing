#include "include/sort-step-by-step.hpp"

SHOW_SORT_STEP::SHOW_SORT_STEP(int size, vector<int> arr) {
    this->size = size;
    this->arr = arr;
}

void SHOW_SORT_STEP::print_array(const vector<int>& arr) {
    for (int value : arr) cout << value << ' ';
    cout << '\n';
}

void SHOW_SORT_STEP::selection_sort(vector<int>& arr) {
    int n = arr.size();
    print_array(arr);

    for (int i = 0; i < n - 1; ++i) {
        int min_index = i;
        for (int j = i + 1; j < n; ++j) {
            if (arr[j] < arr[min_index]) min_index = j;
        }

        if (min_index == i) {
            cout << "Step " << i + 1 << ": no change; " << arr[i]
                 << " is already in the right place.\n";
        } else {
            cout << "Step " << i + 1 << ": change " << arr[i] << " to "
                 << arr[min_index] << ".\n";
            swap(arr[i], arr[min_index]);
        }
        print_array(arr);
    }
}

void SHOW_SORT_STEP::insertion_sort(vector<int>& arr) {
    int n = arr.size();
    print_array(arr);

    for (int i = 1; i < n; ++i) {
        int value = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j] > value) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = value;

        if (j + 1 == i) {
            cout << "Step " << i << ": no change; " << value
                 << " is already in the right place.\n";
        } else {
            cout << "Step " << i << ": move " << value << " to position "
                 << j + 1 << ".\n";
        }
        print_array(arr);
    }
}

void SHOW_SORT_STEP::partition(vector<int>& arr, int low, int high, int& i,
                               int& j, int pivot) {
    while (i <= j) {
        while (i <= high && arr[i] < pivot) ++i;
        while (j >= low && arr[j] > pivot) --j;

        if (i <= j) {
            if (i == j || arr[i] == arr[j]) {
                cout << "Quick sort: no change; " << arr[i]
                     << " is already in place.\n";
            } else {
                cout << "Quick sort: change " << arr[i] << " to " << arr[j]
                     << ".\n";
                swap(arr[i], arr[j]);
                print_array(arr);
            }
            ++i;
            --j;
        }
    }
}

void SHOW_SORT_STEP::quick_sort(vector<int>& arr, int low, int high) {
    int n = arr.size();
    if (low < 0 || high >= n || low >= high) return;

    int pivot = arr[low + (high - low) / 2];
    int i = low;
    int j = high;

    cout << "Quick sort: pivot is " << pivot << ".\n";
    partition(arr, low, high, i, j, pivot);

    if (low < j) quick_sort(arr, low, j);
    if (i < high) quick_sort(arr, i, high);
}

void SHOW_SORT_STEP::merge(vector<int>& arr, int left, int mid, int right) {
    vector<int> merged;
    int i = left;
    int j = mid + 1;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            merged.push_back(arr[i]);
            ++i;
        } else {
            merged.push_back(arr[j]);
            ++j;
        }
    }
    while (i <= mid) {
        merged.push_back(arr[i]);
        ++i;
    }
    while (j <= right) {
        merged.push_back(arr[j]);
        ++j;
    }

    for (int k = left; k <= right; ++k) arr[k] = merged[k - left];
    cout << "Merge: turn this part into sorted order.\n";
    print_array(arr);
}

void SHOW_SORT_STEP::merge_sort(vector<int>& arr, int left, int right) {
    int n = arr.size();
    if (left < 0 || right >= n || left >= right) return;

    int mid = left + (right - left) / 2;
    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

void SHOW_SORT_STEP::heapify(vector<int>& arr, int n, int i) {
    int size = arr.size();
    if (n > size) n = size;
    if (i < 0 || i >= n) return;

    while (true) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n && arr[left] > arr[largest]) largest = left;
        if (right < n && arr[right] > arr[largest]) largest = right;

        if (largest == i) {
            cout << "Heap: no change; " << arr[i] << " stays in place.\n";
            break;
        }

        cout << "Heap: change " << arr[i] << " with " << arr[largest] << ".\n";
        swap(arr[i], arr[largest]);
        print_array(arr);
        i = largest;
    }
}

void SHOW_SORT_STEP::build_heap(vector<int>& arr, int n) {
    int size = arr.size();
    if (n > size) n = size;

    cout << "Build the max heap:\n";
    print_array(arr);

    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(arr, n, i);
    }
}

void SHOW_SORT_STEP::heap_sort(vector<int>& arr) {
    int n = arr.size();
    build_heap(arr, n);

    for (int i = n - 1; i > 0; --i) {
        cout << "Heap sort: move " << arr[0] << " to position " << i << ".\n";
        swap(arr[0], arr[i]);
        heapify(arr, i, 0);
        print_array(arr);
    }
}
