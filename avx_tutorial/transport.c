#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <immintrin.h>

void student_reverse_8xfloat(float *src, float *dst)
{
    __m256 output = _mm256_set1_ps(-1.0); // -1.0 for debugging
    __m256 input = _mm256_loadu_ps(&src[0]);

    __m256 permuted = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7));
    _mm256_storeu_ps(dst, permuted);
}

void print_8xfloat_mem(char *name, float *src)
{
  const int vlen = 8;

  printf("%s = [ ",name);
  for( int vid = 0; vid < vlen; ++vid )
    {
      // if src is less than 0.0f for some reason
      if ( src[vid] < 0.0f )
	printf( " ?, ", src[vid] );
      else
	printf( "%2.f, ", src[vid] );

    }
    printf("]\n");
}

void main() {

    float a[] = {0,1,2,3,4,5,6,7};
    float b[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
    
    student_reverse_8xfloat(a, b);

    print_8xfloat_mem("a", a);
    print_8xfloat_mem("b", b);
}    
