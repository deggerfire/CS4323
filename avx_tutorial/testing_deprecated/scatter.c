#include <immintrin.h>
#include <stdio.h>

void print_m128(__m128 xmm) {
    float values[4];
    _mm_store_ps(values, xmm);

    printf("Contents: [%.2f, %.2f, %.2f, %.2f]\n",
           values[0], values[1], values[2], values[3]);
}

int main() {
    // Initialize your __m256 and __m128 registers
    __m256 m256_register = _mm256_setr_ps(1, 2, 3, 4, 5, 6, 7, 8);
    __m128 m128_registers[7];
    
    // Extract the upper and lower 128-bit lanes from m256_register
    __m128 lower_lane = _mm256_extractf128_ps(m256_register, 0); // Lower 128 bits
    __m128 upper_lane = _mm256_extractf128_ps(m256_register, 1); // Upper 128 bits
    
    // Place each element of m256_register into a different m128 register
    m128_registers[0] = lower_lane;
    m128_registers[1] = upper_lane;
    m128_registers[2] = _mm_setzero_ps(); // Initialize the rest with zeros or other values
    m128_registers[3] = _mm_setzero_ps();
    m128_registers[4] = _mm_setzero_ps();
    m128_registers[5] = _mm_setzero_ps();
    m128_registers[6] = _mm_setzero_ps();
    
    // Now, m128_registers[0] contains the first 128 bits of m256_register,
    // m128_registers[1] contains the second 128 bits, and the rest are zero-initialized.

    print_m128(m128_registers[0]);   
    
    return 0;
}
