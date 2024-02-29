#include <stdlib.h>
#include <stdio.h>

void opt_compute(int n, int* a, int* o);

// FOILer
void compute(int n, int* a, int* o) {
    for (int i = 0; i < n; i++) {
        o[i] = (a[i+0]+a[i+1])*(a[i+2]+a[i+3]);
    }
}

int main() {
    int a[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    int o[20];
    int o1[20];

    int numFails = 0;

    // Calculated using main.py
    int n = 17;

    compute(n, a, o);
    opt_compute(n, a, o1);

    for (int i = 0; i < 17; i++) {
        if (o[i]==o1[i]) printf("PASS\n");
        else {
            printf("FAIL\n");
            numFails++;
        }
    }

    printf("Number of fails: %d\n", numFails);
}