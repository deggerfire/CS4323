/* WORKING CODE 

Takes two 128-bit (int) registers and concatenates them into a 256-bit (int) register.
*/

#include <immintrin.h>
#include <stdio.h>

int main() {
    __m128i xmm1, xmm2;    // Two 128-bit registers
    __m256i ymm;           // One 256-bit register

    // Initialize xmm1 and xmm2 with your data
    xmm1 = _mm_set_epi32(8, 7, 6, 5); // will be printed right to left
    xmm2 = _mm_set_epi32(4, 3, 2, 1); // ditto

    // Use _mm256_insertf128_si256 to put xmm1 and xmm2 into ymm
    ymm = _mm256_insertf128_si256(_mm256_castsi128_si256(xmm1), xmm2, 1);

    // Now ymm contains the concatenated 256-bit result
    int* f = (int*)&ymm;
    for (int i = 0; i < 8; i++) {
        printf("%d ", f[i]); // 5 6 7 8 1 2 3 4
    }
    printf("\n");

    return 0;
}
