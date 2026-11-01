#include <iostream>
using namespace std;

// Quick Sort (Sap xep nhanh - Hoare partition voi pivot o giua)
void quickSort(int a[], int left, int right) {
    int pivot = a[left + (right - left) / 2];
    int i = left, j = right;

    while (i <= j) {
        while (a[i] < pivot) i++;
        while (a[j] > pivot) j--;
        if (i <= j) {
            swap(a[i], a[j]);
            i++;
            j--;
        }
    }

    if (left < j) quickSort(a, left, j);
    if (i < right) quickSort(a, i, right);
}

void printArray(int a[], int n) {
    for (int i = 0; i < n; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}

int main() {
    int n = 8;
    int* a = new int[n]{80, 30, 90, 40, 50, 70, 10, 60};

    cout << "Mang ban dau: ";
    printArray(a, n);

    quickSort(a, 0, n - 1);

    cout << "Sau sap xep:  ";
    printArray(a, n);

    delete[] a;
    return 0;
}
