#include <immintrin.h>  // Include AVX intrinsics header
#include <stdio.h>

float matrix[8][4];
// Fill in your matrix with data

void transpose(float matrix[8][4]) {
    // Step 1: Transpose 4x8 matrix into a temporary 8x4 matrix
    __m256 rows[4];
    for (int i = 0; i < 4; i++) {
        rows[i] = _mm256_loadu_ps(&matrix[0][i * 2]);
        rows[i] = _mm256_insertf128_ps(rows[i], _mm_loadu_ps(&matrix[4][i * 2]), 1);
    }

    // Step 2: Transpose the 8x4 matrix into the final 4x8 transposed matrix
    for (int i = 0; i < 4; i++) {
        __m256 tmp1 = _mm256_unpacklo_ps(rows[i], rows[i + 1]);
        __m256 tmp2 = _mm256_unpackhi_ps(rows[i], rows[i + 1]);
        _mm256_storeu_ps(&matrix[i * 2][0], tmp1);
        _mm256_storeu_ps(&matrix[i * 2][8], tmp2);
        i++;
    }
}


int main() {
    // Define and fill your matrix here
    float matrix[8][4] = {
        {1.0f, 2.0f, 3.0f, 4.0f},
        {5.0f, 6.0f, 7.0f, 8.0f},
        {9.0f, 10.0f, 11.0f, 12.0f},
        {13.0f, 14.0f, 15.0f, 16.0f},
        {17.0f, 18.0f, 19.0f, 20.0f},
        {21.0f, 22.0f, 23.0f, 24.0f},
        {25.0f, 26.0f, 27.0f, 28.0f},
        {29.0f, 30.0f, 31.0f, 32.0f}
    };

    transpose(matrix);

    // The matrix is now transposed
    // You can print it to verify the result
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 4; j++) {
            printf("%.1f ", matrix[i][j]);
        }
        printf("\n");
    }
}
