#include <immintrin.h>

int main() {
    __m256 src1 = _mm256_set_ps(4.0, 3.0, 2.0, 1.0, 8.0, 7.0, 6.0, 5.0);
    __m256 src2 = _mm256_set_ps(12.0, 11.0, 10.0, 9.0, 16.0, 15.0, 14.0, 13.0);

    // Create a control mask for shuffling
    // const int controlMask = 0b10110000; // Shuffle elements 1, 2, 5, and 6 from src1

    const int controlMask = 0b01011000; // Binary: 01011000

    //__m256 result = _mm256_shuffle_ps(src1, src2, controlMask);
    //     __m256 result = _mm256_permutevar8x32_ps(src1, _mm256_set1_epi32(controlMask));


    // Now 'result' contains [7.0, 6.0, 3.0, 2.0, 15.0, 14.0, 11.0, 10.0]
    
    return 0;
}
