#include <stdlib.h>
#include <stdio.h>

void opt_permute(float* src, float* dst);

// FOILer
void permute(float* src, float* dst) {
    dst[0] = src[1];
    dst[1] = src[2];
    dst[2] = src[3];
    dst[3] = src[0];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
}

int main() {
    float a[8] = {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0};

    float o[8];
    float o1[8];

    int numFails = 0;

    permute(a, o);
    opt_permute(a, o1);

    for (int i = 0; i < 17; i++) {
        if (o[i]==o1[i]) printf("PASS\n");
        else {
            printf("FAIL\n");
            numFails++;
        }
    }

    printf("Number of fails: %d\n", numFails);
}