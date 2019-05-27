#include <string.h>

void swap(regDadosI* a, regDadosI* b) {
    Registro t = *a;
    *a = *b;
    *b = t;
}

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
   array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partition (regDadosI* arr, int low, int high) {
    regDadosI pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++) {
        // If current element is smaller than or
        // equal to pivot
        int diff = strcmp(arr[j].chaveBusca,pivot.chaveBusca);

        if (diff < 0) {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }

        else if (diff == 0 && arr[j].byteOffset < pivot.byteOffset) {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }

    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/* The main function that implements QuickSort
    arr[] --> Array to be sorted,
    low  --> Starting index,
    high  --> Ending index */
void quickSort(regDadosI* arr, int low, int high) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}
