/* SEG FAULTING?? */
#include <immintrin.h>
#include <stdio.h>

int main() {
    __m256 a = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    __m256 b = _mm256_set_ps(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f);
    
    // Blend elements from 'a' and 'b' based on the mask (in this case, all elements from 'a')
    __m256 result = _mm256_blend_ps(a, b, 0b00000000);

    // Now, 'result' contains elements from 'a'

    float myValues[8];
    _mm256_store_ps(myValues, result);

    // Print the values
    for (int i = 0; i < 8; i++) {
        printf("Value %d: %f\n", i, myValues[i]);
    }

    return 0;
}
