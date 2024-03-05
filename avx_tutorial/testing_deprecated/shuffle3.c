#include <immintrin.h>
#include <stdio.h>

int main() {
    // Create two __m256i vectors
    __m256 vec1 = _mm256_set_ps(8, 7, 6, 5, 4, 3, 2, 1);
    __m256 vec2 = _mm256_set_ps(16, 15, 14, 13, 12, 11, 10, 9);

    // Shuffle vec1 and vec2 using _mm256_permutevar8x32_epi32
    // how to read this network:
    //- read the elements of vec1, in the idx order provided below
    // try 0b01011000
    __m256 shuffled = _mm256_shuffle_ps(vec1, vec2, _MM_SHUFFLE(3,3,3,3));

    // print contents of shuffled
    float* f = (float*)&shuffled;
    for (int i = 0; i < 8; i++) {
        printf("%.1f ", f[i]);
    }
    printf("\n");

    return 0;
}
