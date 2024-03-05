#include <immintrin.h>
#include <stdio.h>

int main() {
    // Initialize a 256-bit AVX register with data
    __m256 data = _mm256_set_ps(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);

    // Shuffle the elements to get the third and seventh floats into a 128-bit register
    __m256 shuffled = _mm256_permute_ps(data, _MM_SHUFFLE(1, 1, 1, 1));

    // Extract the third and seventh floats using AVX shuffle instructions
    __m128 third_seventh = _mm256_extractf128_ps(shuffled, 0); // Extract the lower 128 bits

    // Create a 64-bit register to store the extracted floats
    __m128d result = _mm_castps_pd(third_seventh); // Convert the 128-bit float register to a 128-bit double register

    // Now, 'result' contains the third and seventh floats as double precision values
    double values[2];
    _mm_store_pd(values, result);
    printf("Third float: %lf\n", values[0]);
    printf("Seventh float: %lf\n", values[1]);

    return 0;
}





// #include <immintrin.h>  // Include AVX intrinsics header
// #include <stdio.h>

// int main() {
//     // Initialize a 256-bit AVX register with 8 floats
//     __m256 avx256 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

//     // Extract the lower 128 bits (4 floats) and store them in a 128-bit AVX register
//     __m128 avx128 = _mm256_extractf128_ps(avx256, 0); // 0 indicates the lower 128 bits

//     // Now you have 4 floats in avx128
//     float result[4];
//     _mm_storeu_ps(result, avx128);

//     // Print the extracted floats
//     printf("%f %f %f %f\n", result[0], result[1], result[2], result[3]);

//     return 0;
// }
