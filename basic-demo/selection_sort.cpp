#include <iostream>
using namespace std;

// Selection Sort (Sap xep chon)
void selectionSort(int a[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j] < a[min_idx]) {
                min_idx = j;
            }
        }
        swap(a[i], a[min_idx]);
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
    int* a = new int[n]{64, 25, 12, 22, 11, 90, 34};

    cout << "Mang ban dau: ";
    printArray(a, n);

    selectionSort(a, n);

    cout << "Sau sap xep:  ";
    printArray(a, n);

    delete[] a;
    return 0;
}
