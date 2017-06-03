#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef int key[3];
#define KEYLEN 3

typedef struct LN {
    key *list;
    int len;
    int max_len;
} LN;

void copyN(key source, key dest) {
    // copies the values from the source key the the dest key
    for (int i = 0; i < 3; i++) {
        dest[i] = source[i];
    }
}

bool eqN(key k1, key k2) {
    // returns a boolean for all values being the same
    return (k1[0] == k2[0] && k1[1] == k2[1] && k1[2] == k2[2]);
}

char *showN(key k) {
    // returns string value of
    char *out;
    out = (char *) malloc(7 + 3*11);
    sprintf(out, "{%d, %d, %d}", k[0], k[1], k[2]);
    return out;
}

void printL(int *l, int length) {
    // print a list with a defined length
    for (int i = 0; i < length; i++) {
        printf("%d ", l[i]);
    }
    printf("\n");
}

LN *init_array(void) {
    // Initialise LN array and return to a pointer
    LN *array;
    array = (LN *) malloc(sizeof(LN));
    array->max_len = 20;
    array->len = 0;
    array->list = (key *) malloc(sizeof(key) * 20); // my calcs say 15 but 20 :)
    return array;
}

void printLN(LN *ln) {
    // prints an LN array
    for (int i = 0; i < ln->len; i++) {
        printL(ln->list[i], 3);
    }
    printf("\n");
}

int distance_2(key k1, key k2) {
    // returns the square of the euclidian distance
    // squared distance ensures that out put can be an int whilst still
    // testtaining order by distance
    int diff[2];
    diff[0] = k1[1] - k2[1];
    diff[1] = k1[2] - k2[2];
    return diff[0] * diff[0] + diff[1] * diff[1];
}

bool lt(key self_key, key k1, key k2) {
    // returns a bool that gets used to rank keys for the quicksort
    int d1 = distance_2(k1, self_key);
    int d2 = distance_2(k2, self_key);
    if (d1 == d2) {
        // The series of conditional controls allow the uniq function to operate
        // correctly with O(N) time complexity
        if (k1[0] != k2[0]) {
            return k1[0] > k2[0]; // rank by the reading generated
        } else if (k1[1] != k2[1]) {
            return k1[1] < k2[1]; // else rank by the row coordinate
        } else {
            return k1[2] < k2[2]; // then else rank by the column coordinate
        }
    } else {
        return d1 < d2;
    }
}

void append(LN *self, key n) {
    // makes a copy of key n and stores it at the end of the LN array
    if (self->len == self->max_len) {
        // double the size of the allocated memory if needed
        self->max_len =  2 * self->max_len;
        self->list = (key *) realloc(self->list, sizeof(key) * self->max_len);
    }
    copyN(n, self->list[self->len]);
    self->len++;
}

void uniq(LN *ln) {
    // removes duplicate elements from the LN array
    // NOTE must be used with a sorted LN array
    int to = 0;
    for (int from = 0; from < ln->len; from++) {
        if (!(eqN(ln->list[from], ln->list[to]))) {
            // copies the next unique key next to the key it's being tested against
            to++;
            if (from != to) {
                copyN(ln->list[from], ln->list[to]);
            }
        }
    }
    // set the metadata of the LN array an reallocate the assigned memory
    ln->len = to + 1;
    ln->max_len = ln->len;
    ln->list = (key *) realloc(ln->list, sizeof(key) * ln->max_len);
}

void swap_ints(int *a, int *b) {
    // simple integer swap that swaps the values at the memory addresses
    int temp = *a;
    *a = *b;
    *b = temp;
}

void swap_keys(key k1, key k2) {
    // swaps the values of the two keys element by element
    for (int i = 0; i < 3; i++) {
        swap_ints(&k1[i], &k2[i]);
    }
}

void quicksort(key self_key, key *A, int len) {
    // A modified version of the quicksort algorithm hosted on rosettacode.org
    // using the middlie value as the pivot each time
    if (len < 2) return;
    key pivot;
    copyN(A[len / 2], pivot);
    int i, j;
    for (i = 0, j = len - 1; ; i++, j--)
    {
        while (lt(self_key, A[i], pivot)) i++;
        while (lt(self_key, pivot, A[j])) j--;
        if (i >= j) break;
        swap_keys(A[i], A[j]);
    }

    // uses pointer arithmetic to select portion of list to sort in place
    quicksort(self_key, A, i);
    quicksort(self_key, A + i, len - i);
}

bool event_verify(LN *self) {
    // returns true if the first 5 readings of the LN array are 1 or higher(D!)
    if (self->len > 4) {
        for (int i = 0; i < 5; i++) {
            if (self->list[i][0] < 1) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

int test(int argc, char const *argv[]) {
    LN *array1 = init_array();
    key temp_array[] = {
        {4, 5, 7}, {65, 6, 7}, {83, 3, 7}, {-1, -1, -1}, {0, 0, 0},
        {2, 6, 3}, {83, 5, 5}, {83, 3, 7}, {782, 46, 2}, {1, 1, 1}
    };

    for (int i = 0; i < 10; i++) {
        printL(temp_array[i], 3);
    }
    printf("\n");
    for (int i = 0; i < 10; i++) {
        append(array1, temp_array[i]);
    }
    printLN(array1);
    key temp_key = {83, 3, 7};
    append(array1, temp_key);
    printLN(array1);
    int long_array[7];
    for (int i = 0; i < 7; i++) {
        long_array[i] = i*i;
    }
    printL(long_array, 7);
    swap_ints(&long_array[2], &long_array[4]);
    printL(long_array, 7);

    swap_keys(array1->list[2], array1->list[4]);
    printLN(array1);
    quicksort(array1->list[0], array1->list, array1->len);
    printLN(array1);
    uniq(array1);
    printLN(array1);
    key temp_array2[] = {
        {1, 3, 1}, {1, 2, 1}, {1, 3, 0}, {1, 3, 2},
        {0, 2, 0}, {1, 2, 2}, {0, 1, 1}, {1, 3, 3}
    };
    LN *array2 = init_array();
    for (int i = 0; i < 8; i++) {
        append(array2, temp_array2[i]);
    }
    printLN(array2);
    key ref_key2 = {1, 3, 1};
    quicksort(ref_key2, array2->list, array2->len);

    printf("after quicksort\n");
    printLN(array2);


    printf("event detection for array1: %d\n", event_verify(array1));
    return 0;
}
