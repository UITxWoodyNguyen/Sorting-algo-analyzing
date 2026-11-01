#include <iostream>
using namespace std;

// Heap Sort (Sap xep vun dong)
void heapify(int a[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && a[left] > a[largest])
        largest = left;

    if (right < n && a[right] > a[largest])
        largest = right;

    if (largest != i) {
        swap(a[i], a[largest]);
        heapify(a, n, largest);
    }
}

void heapSort(int a[], int n) {
    // Xay dung Max-Heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(a, n, i);
    }

    // Trich xuat tung phan tu tu heap
    for (int i = n - 1; i > 0; i--) {
        swap(a[0], a[i]);
        heapify(a, i, 0);
    }
}

void printArray(int a[], int n) {
    for (int i = 0; i < n; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}

int main() {
    int n = 7;
    int* a = new int[n]{4, 10, 3, 5, 1, 99, 23};

    cout << "Mang ban dau: ";
    printArray(a, n);

    heapSort(a, n);

    cout << "Sau sap xep:  ";
    printArray(a, n);

    delete[] a;
    return 0;
}
