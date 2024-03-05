#include <immintrin.h>
#include <stdio.h>

int main() {
    float base[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    int indices[] = {2, 0, 3, 1};
    __m256i vindex = _mm256_loadu_si256((__m256i*)indices);
    __m256 result = _mm256_i32gather_ps(base, vindex, 4);

    float resultArray[8];
    _mm256_storeu_ps(resultArray, result);

    for (int i = 0; i < 8; i++) {
        printf("result[%d] = %.2f\n", i, resultArray[i]);
    }
}
