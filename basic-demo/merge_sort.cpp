#include <iostream>
using namespace std;

// Merge Sort (Sap xep tron)
void merge(int a[], int left, int mid, int right, int temp[]) {
    int i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right) {
        if (a[i] <= a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
    }

    while (i <= mid) temp[k++] = a[i++];
    while (j <= right) temp[k++] = a[j++];

    for (int p = left; p <= right; p++) {
        a[p] = temp[p];
    }
}

void mergeSort(int a[], int left, int right, int temp[]) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSort(a, left, mid, temp);
    mergeSort(a, mid + 1, right, temp);
    merge(a, left, mid, right, temp);
}

void printArray(int a[], int n) {
    for (int i = 0; i < n; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}

int main() {
    int n = 7;
    int* a = new int[n]{38, 27, 43, 3, 9, 82, 10};
    int* temp = new int[n]; // Mang dong phu dung de tron

    cout << "Mang ban dau: ";
    printArray(a, n);

    mergeSort(a, 0, n - 1, temp);

    cout << "Sau sap xep:  ";
    printArray(a, n);

    delete[] a;
    delete[] temp;
    return 0;
}
