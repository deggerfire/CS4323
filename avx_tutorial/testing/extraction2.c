#include <immintrin.h> // Include the Intel Intrinsics header
#include <stdio.h>

int main() {
    // Declare a 256-bit AVX register containing 8 single-precision floats
    __m256 myAvxRegister = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

    // Extract the lower 128 bits of the AVX register into a 128-bit register
    __m128 my128BitRegister = _mm256_extractf128_ps(myAvxRegister, 0); // 0 indicates the lower 128 bits

    // Extract a single float from the 128-bit register
    float myFloat = _mm_cvtss_f32(my128BitRegister); // Extract the first (0th) float

    // Print the extracted float
    printf("Extracted Float: %f\n", myFloat);

    return 0;
}
