/* WORKING CODE 

Takes two 128-bit (float) registers and concatenates them into a 256-bit (float) register.
*/

#include <immintrin.h>
#include <stdio.h>

int main() {
    __m128 xmm1, xmm2;    // Two 128-bit registers
    __m256 ymm;           // One 256-bit register

    // Initialize xmm1 and xmm2 with your data
    xmm1 = _mm_set_ps(8, 7, 6, 5); // will be printed right to left
    xmm2 = _mm_set_ps(4, 3, 2, 1); // ditto

    // Use _mm256_insertf128_si256 to put xmm1 and xmm2 into ymm
    ymm = _mm256_insertf128_ps(_mm256_castps128_ps256(xmm1), xmm2, 1);

    // Now ymm contains the concatenated 256-bit result
    float* f = (float*)&ymm;
    for (int i = 0; i < 8; i++) {
        printf("%.1f ", f[i]); // 5 6 7 8 1 2 3 4
    }
    printf("\n");

    return 0;
}
