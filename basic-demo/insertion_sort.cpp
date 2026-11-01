#include <iostream>
using namespace std;

// Insertion Sort (Sap xep chen)
void insertionSort(int a[], int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
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
    int* a = new int[n]{12, 11, 13, 5, 6, 45, 2};

    cout << "Mang ban dau: ";
    printArray(a, n);

    insertionSort(a, n);

    cout << "Sau sap xep:  ";
    printArray(a, n);

    delete[] a;
    return 0;
}
