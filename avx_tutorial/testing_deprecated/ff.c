#include <stdio.h>
#include <immintrin.h>

int main() {
    __m256 vectorA = _mm256_set_ps(4.0f, 3.0f, 2.0f, 1.0f, 8.0f, 7.0f, 6.0f, 5.0f);
    __m256 vectorB = _mm256_set_ps(12.0f, 11.0f, 10.0f, 9.0f, 16.0f, 15.0f, 14.0f, 13.0f);

    // Shuffle elements from vectorA and vectorB to create a new vector
    // 0b 01 01 01 00
    __m256 result = _mm256_shuffle_ps(vectorA, vectorB, 0b01010100);

    // Extract the elements from the result vector
    float resultArray[8];
    _mm256_storeu_ps(resultArray, result);

    // Print the elements
    for (int i = 0; i < 8; i++) {
        printf("result[%d] = %f\n", i, resultArray[i]);
    }

    return 0;
}
