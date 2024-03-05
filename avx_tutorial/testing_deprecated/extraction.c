/* WORKING CODE */
#include <immintrin.h> // Include the Intel Intrinsics header
#include <stdio.h>

int main() {
    // Declare a 256-bit AVX register containing 8 single-precision floats
    // 256 bits = 8 floats
    __m256 myAvxRegister = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

    // Extract the lower 128 bits of the AVX register into a new 128-bit register
    // 128 bits = 4 floats
    __m128 my128BitRegister = _mm256_extractf128_ps(myAvxRegister, 1); // 0 for lower 128 bits, 1 for upper

    // Now, you can work with the 128-bit register as needed
    float myValues[4];
    _mm_storeu_ps(myValues, my128BitRegister);

    // Print the values
    for (int i = 0; i < 4; i++) {
        printf("Value %d: %f\n", i, myValues[i]);
    }

    return 0;
}
