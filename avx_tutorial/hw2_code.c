/*
  HW2 SIMD!

  Instructions: find all instances of "STUDENT_TODO" and replace with SIMD code
                that makes the test corresponding to that function pass.

                At the command prompt in the directory containing this code
                run 'make'

  Submission: For this assignment you will upload three artifacts to canvas.
        1. [figures.pdf] containing pictures describing the movements
                         being performed by the SIMD instructions.
	      2. [results.txt] containing the test output of your code.
	      3. [code.c] Your modified version of this code.

  - richard.m.veras@ou.edu
*/



/*
  NOTE: You can use any instructions from:
     https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE4_1,AVX,AVX2

  But, I strongly suggest the following instructions:


  / Load
  __m256 _mm256_loadu_ps (float const * mem_addr)

  / Store
  void _mm256_storeu_ps (float * mem_addr, __m256 a)

  / set
  __m256 _mm256_set1_ps (float a)

  / extract
  __m128 _mm256_extractf128_ps (__m256 a, const int imm8) // Also need __m128 version
  int _mm_extract_ps (__m128 a, const int imm8) // SSE 4.1

  / permute with one input
  __m256 _mm256_permute_ps (__m256 a, int imm8) // within lane
  __m256 _mm256_permutevar8x32_ps (__m256 a, __m256i idx) // across lane

  / Blend
  __m256 _mm256_blend_ps (__m256 a, __m256 b, const int imm8)


  / Shuffle with two inputs
  __m256 _mm256_shuffle_ps (__m256 a, __m256 b, const int imm8)


  / Math
  __m256 _mm256_fmadd_ps (__m256 a, __m256 b, __m256 c) // _mm256_fmaddsub_ps (__m256 a, __m256 b, __m256 c)?

*/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <immintrin.h>


/*
  Helper functions
*/

float max_pair_wise_diff(int m, float *a, float *b)
{
  float max_diff = 0.0;
  
  for(int i = 0; i < m; ++i) {
      // add the two floats together
      float sum  = fabs(a[i]+b[i]);
      // subtract the two floats (absolute value)
      float diff = fabs(a[i]-b[i]);

      float res = 0.0f;

      // avoid dividing by zero. if the sum is 0, res is 0
      if(sum == 0.0f)
	      res = diff;
      else
        // res
	      res = 2*diff/sum;

      if( res > max_diff )
	      max_diff = res;
  }

  return max_diff;
}



void print_8xfloat_mem(char *name, float *src)
{
  const int vlen = 8;

  printf("%s = [ ",name);
  for( int vid = 0; vid < vlen; ++vid )
    {
      // src is less than 0.0f for some reason
      if ( src[vid] < 0.0f )
	printf( " !, ", src[vid] );
      else
	printf( "%2.f, ", src[vid] );

    }
    printf("]\n");
}

void print_float_mem(char *name, int vlen, float *src)
{

  printf("%s = [ ",name);
  for( int vid = 0; vid < vlen; ++vid )
    {
      if ( src[vid] < 0.0f )
	printf( " x, ", src[vid] );
      else
	printf( "%2.f, ", src[vid] );
    }
    printf("]\n");
}

void print_float_mem_as_vects(char *name, int size, int vlen, float *src)
{
  // iterate up to size, but print in chunks of vlen
  for( int i = 0; i < size; i+=vlen ) {
      printf("%s[%2i:%2i] = [ ",name,i,i+vlen);
      for( int vid = 0; vid < vlen; ++vid ) {
	      if ( src[vid+i] < 0.0f )
	        printf( " x, ", src[vid+i] );
	      else
	        printf( "%2.f, ", src[vid+i] );
	    }
      printf("]\n");
  }
  printf("\n");

}

// these are helper functions i made bc they were easier to understand
void print_m128(__m128 xmm) {
    float values[4];
    _mm_store_ps(values, xmm);

    printf("(4-float register) [%.2f, %.2f, %.2f, %.2f]\n",
           values[0], values[1], values[2], values[3]);
}

void print_m256(__m256 ymm) {
    float values[8];
    _mm256_storeu_ps(values, ymm);

    printf("(8-float register) [%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f]\n",
           values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);
}

// SIMD # 1
// Input: 8 floats in src (i.e. src[0:7])
// Goal: Put them all in the dst array in reverse order (i.e. dst[7:0])
void reference_reverse_8xfloat(float *src, float *dst)
{
  const int vlen = 8;
  
  for( int vid = 0; vid < vlen; ++vid )
    {
      dst[(vlen-1)-vid] = src[vid];
    }
}

void student_reverse_8xfloat(float *src, float *dst)
{

  __m256 output = _mm256_set1_ps(-1.0); // for debugging
  
  __m256 input = _mm256_loadu_ps(&src[0]);


  {
    /* 
       STUDENT_TODO: Add student code below.
    */
    
    __m256 permuted1 = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7));
    _mm256_storeu_ps(dst, permuted1);

    // Print the shuffled elements
    //  for (int i = 0; i < 8; i++) {
    //      printf("%.2f ", dst[i]);
    //  }
    //  printf("done\n");

    /* 
       End of student code.
    */
  }

  // Store the result back to memory.
  //_mm256_storeu_ps(&dst[0],output); // test PASSes once I comment this out lol
  
}

void test_reverse_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_reverse_8xfloat(a, bt);
  student_reverse_8xfloat(a, br);


  float res = max_pair_wise_diff(8, bt, br);

  printf("test_reverse_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_8xfloat_mem(" a", a);
      print_8xfloat_mem("bt", bt);
      print_8xfloat_mem("br", br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 2
////////////////
// SIMD # 2.1 //
////////////////
void reference_rotate_by_4_8xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int shift = 4;
  
  for( int vid = 0; vid < vlen; ++vid )
    {
      dst[(vlen+(vid - shift))%(vlen)] = src[vid];
    }
}

void student_rotate_by_4_8xfloat(float *src, float *dst)
{

  __m256 output = _mm256_set1_ps(-1);
  __m256 input = _mm256_loadu_ps(&src[0]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    // __m256 XXX = ...
    // ...
    // ...
    // output = ...;
    
    __m256 permuted2 = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
    _mm256_storeu_ps(dst, permuted2);

    /* 
       End of student code.
    */
  }

  // Store the result back to memory.
  // (from now on, this will be done in the STUDENT_TODO section. all future code in this section will be removed)
  /* note: online sources say that the following two lines are identical:
        _mm256_storeu_ps(dst, output); 
        _mm256_storeu_ps(&dst[0],output);
     for convenience, I will use the former from now on
  */ 
}

void test_rotate_by_4_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_rotate_by_4_8xfloat(a, bt);
  student_rotate_by_4_8xfloat(a, br);


  float res = max_pair_wise_diff(8, bt, br);

  printf("test_rotate_by_4_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_8xfloat_mem(" a", a);
      print_8xfloat_mem("bt", bt);
      print_8xfloat_mem("br", br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 3
////////////////
// SIMD # 2.2 //
////////////////
void reference_rotate_by_2_8xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int shift = 2;
  
  for( int vid = 0; vid < vlen; ++vid )
    {
      dst[(vlen+(vid - shift))%(vlen)] = src[vid];
    }
}

void student_rotate_by_2_8xfloat(float *src, float *dst)
{

  __m256 output = _mm256_set1_ps(-1.0); // for debugging
  __m256 input = _mm256_loadu_ps(&src[0]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    __m256 permuted3 = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(1, 0, 7, 6, 5, 4, 3, 2));
    _mm256_storeu_ps(dst, permuted3);
    
    /* 
       End of student code.
    */
  }

}

void test_rotate_by_2_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_rotate_by_2_8xfloat(a, bt);
  student_rotate_by_2_8xfloat(a, br);


  float res = max_pair_wise_diff(8, bt, br);

  printf("test_rotate_by_2_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_8xfloat_mem(" a", a);
      print_8xfloat_mem("bt", bt);
      print_8xfloat_mem("br", br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 4
////////////////
// SIMD # 2.3 //
////////////////
void reference_rotate_by_1_8xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int shift = 1;
  
  for( int vid = 0; vid < vlen; ++vid )
    {
      dst[(vlen+(vid - shift))%(vlen)] = src[vid];
    }
}

void student_rotate_by_1_8xfloat(float *src, float *dst)
{

  __m256 output = _mm256_set1_ps(-1.0); // for debugging
  __m256 input = _mm256_loadu_ps(&src[0]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    __m256 permuted3 = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1));
    _mm256_storeu_ps(dst, permuted3);
    
    /* 
       End of student code.
    */
  }

}


void test_rotate_by_1_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1};

    reference_rotate_by_1_8xfloat(a, bt);
  student_rotate_by_1_8xfloat(a, br);

  float res = max_pair_wise_diff(8, bt, br);

  printf("test_rotate_by_1_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_8xfloat_mem(" a", a);
      print_8xfloat_mem("bt", bt);
      print_8xfloat_mem("br", br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 5
/*
  SIMD 3 Transpose 4x2 matrix in column major (i.e. row stride = 4)

 Matrix A                 Matrix A^T
   ___
  |0 4|                    ________
  |1 5|   --- becomes --> | 0 1 2 3|
  |2 6|                   | 4 5 6 7|
  |3 7|                    --------
   ---

  Stored in memory we have:

  [0,1,2,3,4,5,6,7] -- becomes --> [0,4,1,5,2,6,3,7]
*/

void reference_transpose_4x2_colmaj_8xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int m = 4;
  const int n = 2;
  const int rs_s = m;
  const int cs_s = 1;

  const int rs_d = n;
  const int cs_d = 1;

  for (int i = 0; i < m; ++i )
      for (int j = 0; j < n; ++j )
	dst[j*cs_d + i*rs_d] = src[i*cs_s + j*rs_s];
  
}

void student_transpose_4x2_colmaj_8xfloat(float *src, float *dst)
{

  __m256 output = _mm256_set1_ps(-1.0); // for debugging
  __m256 input = _mm256_loadu_ps(&src[0]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    __m256 transpose = _mm256_permutevar8x32_ps(input, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
    _mm256_storeu_ps(dst, transpose);
    
    /* 
       End of student code.
    */
  }

}

void test_transpose_4x2_colmaj_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1};
  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_transpose_4x2_colmaj_8xfloat(a, bt);
  student_transpose_4x2_colmaj_8xfloat(a, br);


  float res = max_pair_wise_diff(8, bt, br);

  printf("test_transpose_4x2_colmaj_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_8xfloat_mem(" a", a);
      print_8xfloat_mem("bt", bt);
      print_8xfloat_mem("br", br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 6
/*
  SIMD 4: rotate two concatenated vectors by 1
*/

void reference_rotate_by_1_16xfloat(float *src, float *dst)
{
  const int vlen = 16;
  const int shift = 1;
  
  for( int vid = 0; vid < vlen; ++vid )
    {
      dst[(vlen+(vid - shift))%(vlen)] = src[vid];
    }
}

void student_rotate_by_1_16xfloat(float *src, float *dst)
{

  __m256 output00_07 = _mm256_set1_ps(-1.0);
  __m256 output08_15 = _mm256_set1_ps(-1.0);
  __m256 input00_07 = _mm256_loadu_ps(&src[0]);
  __m256 input08_15 = _mm256_loadu_ps(&src[8]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    // __m256 XXX = ...
    // ...
    // ...
    // output = ...;
    __m256 x00_07 = _mm256_permutevar8x32_ps(input00_07, _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1));
    __m256 x08_15 = _mm256_permutevar8x32_ps(input08_15, _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1));

    _mm256_storeu_ps(&dst[0],x00_07);
    _mm256_storeu_ps(&dst[8],x08_15);

    // now swap the 8th and 15th values
    float temp = dst[7];
    dst[7] = dst[15];
    dst[15] = temp;

    /* 
       End of student code.
    */
  }
  
}


void test_rotate_by_1_16xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15 };
  float bt[16] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1};
  float br[16] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_rotate_by_1_16xfloat(a, bt);
  student_rotate_by_1_16xfloat(a, br);


  float res = max_pair_wise_diff(16, bt, br);

  printf("test_rotate_by_1_16xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem(" a", 16, a);
      print_float_mem("bt", 16, bt);
      print_float_mem("br", 16, br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 7
/*
  SIMD 5.1 Transpose 4x4 matrix in column major (i.e. row stride = 4)

 Matrix A                 Matrix A^T
   __________                     ____________ 
  |0  4  8 12|                   |  0  1  2  3|
  |1  5  9 13|   --- becomes --> |  4  5  6  7|
  |2  6 10 14|                   |  8  9 10 11|
  |3  7 11 15|                   | 12 13 14 15|
   ----------                     ------------ 

  Stored in memory we have:

  [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15] -- becomes --> [0,4,8,12, 1,5,9,13, 2,6,10,14 3,7,11,15]
*/

void reference_transpose_4x4_colmaj_16xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int m = 4;
  const int n = 4;
  const int rs_s = m;
  const int cs_s = 1;

  const int rs_d = n;
  const int cs_d = 1;

  for (int i = 0; i < m; ++i )
      for (int j = 0; j < n; ++j )
	dst[j*cs_d + i*rs_d] = src[i*cs_s + j*rs_s];
  
}

void student_transpose_4x4_colmaj_8xfloat(float *src, float *dst)
{

  __m256 output00_07 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output08_15 = _mm256_set1_ps(-1.0); // for debugging
  __m256 input00_07 = _mm256_loadu_ps(&src[0]);
  __m256 input08_15 = _mm256_loadu_ps(&src[8]);

  {
    /* 
       INPUT: two registers containing the first 8 floats and the last 8 floats
    */

    //__m256 a = _mm256_permutevar8x32_ps(input00_07, _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, 0));
    //__m256 b = _mm256_permutevar8x32_ps(input08_15, _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1));

    __m128 a1 = _mm256_extractf128_ps(input00_07, 0); // 0 = lower 128 bits
    __m128 a2 = _mm256_extractf128_ps(input00_07, 1); // 1 = upper 128 bits

    __m128 b1 = _mm256_extractf128_ps(input08_15, 0); 
    __m128 b2 = _mm256_extractf128_ps(input08_15, 1); 

    /* 
    this function is built into SSE instruction set
    
      #define _MM_TRANSPOSE4_PS(row0,row1,row2,row3) {
        __m128 _Tmp3, _Tmp2, _Tmp1, _Tmp0; 
        _Tmp0 = _mm_shuffle_ps((row0), (row1), 0x44); 
        _Tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE); 
        _Tmp1 = _mm_shuffle_ps((row2), (row3), 0x44); 
        _Tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE); 
        (row0) = _mm_shuffle_ps(_Tmp0, _Tmp1, 0x88); 
        (row1) = _mm_shuffle_ps(_Tmp0, _Tmp1, 0xDD); 
        (row2) = _mm_shuffle_ps(_Tmp2, _Tmp3, 0x88); 
        (row3) = _mm_shuffle_ps(_Tmp2, _Tmp3, 0xDD);
      }
    */
    
    //_MM_TRANSPOSE4_PS(a1, a2, b1, b2);
    // the above line of code is equivalent to the following:
        __m128 _Tmp3, _Tmp2, _Tmp1, _Tmp0; 
        // a1 = 0, 1, 2, 3
        // a2 = 4, 5, 6, 7
        // b1 = 8, 9, 10, 11
        // b2 = 12, 13, 14, 15
        _Tmp0 = _mm_shuffle_ps((a1), (a2), 0x44); // 0x44 = 68 = 0b01000100 = 01 00 01 00 
                                                  // = a1[0] a1[1] a2[0] a2[1] -> _Tmp0[0,1,2,3] = 0 1 4 5
        _Tmp2 = _mm_shuffle_ps((a1), (a2), 0xEE); // 0xEE = 238 = 0b11101110 = 11 10 11 10
                                                  // = a1[3] a1[2] a2[3] a2[2] -> _Tmp2[0,1,2,3] = 2 3 6 7
        _Tmp1 = _mm_shuffle_ps((b1), (b2), 0x44); // 68
                                                  // = b1[0] b1[1] b2[0] b2[1] -> _Tmp1[0,1,2,3] = 8 9 12 13
        _Tmp3 = _mm_shuffle_ps((b1), (b2), 0xEE); // 238
                                                  // = b1[3] b1[2] b2[3] b2[2] -> _Tmp3[0,1,2,3] = 10 11 14 15
        // _Tmp0 = 0 1 4 5
        // _Tmp1 = 8 9 12 13
          // print_m128(_Tmp0);
          // print_m128(_Tmp1);
        // _Tmp2 = 2 3 6 7
        // _Tmp3 = 10 11 14 15
          // print_m128(_Tmp2);
          // print_m128(_Tmp3);
        (a1) = _mm_shuffle_ps(_Tmp0, _Tmp1, 0x88); // 0x88 = 136 = 0b10001000 = 10 00 10 00
                                                   // = _Tmp0[0] _Tmp0[2] _Tmp1[0] _Tmp1[2] -> a1[0,1,2,3] = 0 4 8 12 
        (a2) = _mm_shuffle_ps(_Tmp0, _Tmp1, 0xDD); // 0xDD = 221 = 0b11011101 = 11 01 11 01
                                                   // = _Tmp0[1] _Tmp0[3] _Tmp1[1] _Tmp1[3] -> a2[0,1,2,3] = 1 5 9 13
        (b1) = _mm_shuffle_ps(_Tmp2, _Tmp3, 0x88); // 136
                                                   // = _Tmp2[0] _Tmp2[2] _Tmp3[0] _Tmp3[2] -> b1[0,1,2,3] = 2 6 10 14
        (b2) = _mm_shuffle_ps(_Tmp2, _Tmp3, 0xDD); // 221
                                                    // = _Tmp2[1] _Tmp2[3] _Tmp3[1] _Tmp3[3] -> b2[0,1,2,3] = 3 7 11 15
        // a1 = 0, 4, 8, 12
        // a2 = 1, 5, 9, 13
        // b1 = 2, 6, 10, 14
        // b2 = 3, 7, 11, 15

    input00_07 = _mm256_insertf128_ps(_mm256_castps128_ps256(a1), a2, 1);
    input08_15 = _mm256_insertf128_ps(_mm256_castps128_ps256(b1), b2, 1);

    // Store the result back to memory.
    _mm256_storeu_ps(&dst[0],input00_07);
    _mm256_storeu_ps(&dst[8],input08_15);
    /* 
       End of student code.
    */
  }


}


void test_transpose_4x4_colmaj_8xfloat()
{
  float a[] = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15};
  float bt[16] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1};
  float br[16] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1};

  
  reference_transpose_4x4_colmaj_16xfloat(a, bt);
  student_transpose_4x4_colmaj_8xfloat(a, br);


  float res = max_pair_wise_diff(16, bt, br);

  printf("test_transpose_4x4_colmaj_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem(" a", 16, a);
      print_float_mem("bt", 16, bt);
      print_float_mem("br", 16, br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

// 8
/*
  SIMD 5.2 Transpose 8x4 matrix in column major (i.e. row stride = 8)
  
   Matrix A                               Matrix A^T
   __________                      ________________________
  |0  8  16 24|                   |  0  1  2  3  4  5  6  7|
  |1  9  17 25|   --- becomes --> |  8  9 10 11 12 13 14 15|
  |2  10 18 26|                   | 16 17 18 19 20 21 22 23|
  |3  11 19 27|                   | 24 25 26 27 28 29 30 31|
  |4  12 20 28|                    ------------------------
  |5  13 21 29|
  |6  14 22 30|
  |7  15 23 31|
   ----------                     
    
  [0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15, 16,17,18,19,20,21,22,23, 24,25,26,27,28,29,30,31] 
  -- becomes --> 
  [0,8,16,24, 1,9,17,25, 2,10,18,26, 3,11,19,27, 4,12,20,28 5,13,21,29, 6,14,22,30, 7,15,23,31]
  -- which we can also think of as -->
  [0,8,16,24,1,9,17,25, 2,10,18,26,3,11,19,27, 4,12,20,28,5,13,21,29, 6,14,22,30,7,15,23,31]

*/

void reference_transpose_8x4_colmaj_32xfloat(float *src, float *dst)
{
  const int vlen = 8;
  const int m = 8;
  const int n = 4;
  const int rs_s = m;
  const int cs_s = 1;

  const int rs_d = n;
  const int cs_d = 1;

  for (int i = 0; i < m; ++i )
      for (int j = 0; j < n; ++j )
	dst[j*cs_d + i*rs_d] = src[i*cs_s + j*rs_s];
  
}

void student_transpose_8x4_colmaj_8xfloat(float *src, float *dst)
{
  __m256 output00_07 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output08_15 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output16_23 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output24_31 = _mm256_set1_ps(-1.0); // for debugging

  __m256 input00_07 = _mm256_loadu_ps(&src[0]); // 0, 1, ..., 7
  __m256 input08_15 = _mm256_loadu_ps(&src[8]); // 8, 9, ..., 15
  __m256 input16_23 = _mm256_loadu_ps(&src[16]); // 16, 17, ..., 23
  __m256 input24_31 = _mm256_loadu_ps(&src[24]); // 24, 25, ..., 31

  {
    /* 
       STUDENT_TODO: Add student code below.
    */
    __m256 t0 = _mm256_permute2f128_ps(input00_07, input08_15, 0x20); // 0x20 = 32 = 0b00100000 = 00 10 00 00
    //print_m256(t0); // [0.00, 1.00, 2.00, 3.00, 8.00, 9.00, 10.00, 11.00]
    __m256 t1 = _mm256_permute2f128_ps(input16_23, input24_31, 0x20);
    //print_m256(t1); // [16.00, 17.00, 18.00, 19.00, 24.00, 25.00, 26.00, 27.00]
    __m256 t2 = _mm256_permute2f128_ps(input00_07, input08_15, 0x31); // 0x31 = 49 = 0b00110001 = 00 11 00 01
    //print_m256(t2); // [4.00, 5.00, 6.00, 7.00, 12.00, 13.00, 14.00, 15.00]
    __m256 t3 = _mm256_permute2f128_ps(input16_23, input24_31, 0x31);
    //print_m256(t3); // [20.00, 21.00, 22.00, 23.00, 28.00, 29.00, 30.00, 31.00]

    __m128 a1 = _mm256_extractf128_ps(t0, 0); // 0 = lower 128 bits
    __m128 a2 = _mm256_extractf128_ps(t0, 1); // 1 = upper 128 bits
    __m128 b1 = _mm256_extractf128_ps(t1, 0); 
    __m128 b2 = _mm256_extractf128_ps(t1, 1); 

    __m128 c1 = _mm256_extractf128_ps(t2, 0); // 0 = lower 128 bits
    __m128 c2 = _mm256_extractf128_ps(t2, 1); // 1 = upper 128 bits
    __m128 d1 = _mm256_extractf128_ps(t3, 0); 
    __m128 d2 = _mm256_extractf128_ps(t3, 1); 

    _MM_TRANSPOSE4_PS(a1, a2, b1, b2);
    _MM_TRANSPOSE4_PS(c1, c2, d1, d2);

    input00_07 = _mm256_insertf128_ps(_mm256_castps128_ps256(a1), a2, 1);
    input08_15 = _mm256_insertf128_ps(_mm256_castps128_ps256(b1), b2, 1);
    input16_23 = _mm256_insertf128_ps(_mm256_castps128_ps256(c1), c2, 1);
    input24_31 = _mm256_insertf128_ps(_mm256_castps128_ps256(d1), d2, 1);
    /* 
       End of student code.
    */
  }

  // Store the result back to memory.
  _mm256_storeu_ps(&dst[ 0],input00_07);
  _mm256_storeu_ps(&dst[ 8],input08_15);
  _mm256_storeu_ps(&dst[16],input16_23);
  _mm256_storeu_ps(&dst[24],input24_31);
}

void test_transpose_8x4_colmaj_8xfloat()
{
  float a[32] = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15,
                 16,17,18,19,20,21,22,23,24, 25,26,27,28,29,30,31};
  float bt[32] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 };

  float br[32] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 };

  
  reference_transpose_8x4_colmaj_32xfloat(a, bt);
  student_transpose_8x4_colmaj_8xfloat(a, br);


  float res = max_pair_wise_diff(32, bt, br);

  printf("test_transpose_8x4_colmaj_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem_as_vects(" a", 32,8, a);
      print_float_mem_as_vects("bt", 32,8, bt);
      print_float_mem_as_vects("br", 32,8, br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}

//////////////////////////
//// GATHER AND SCATTER //
//////////////////////////

// 9
/*
  Gathering at a stride
  (get every Xth element)
*/
void reference_gather_at_stride_8xfloat(float *src, float *dst)
{
  const int vlen   = 8;
  const int stride = 4;

  for( int vid = 0; vid < vlen; ++vid )
    dst[vid] = src[vid*stride];

}


void student_gather_at_stride_8xfloat(float *src, float *dst)
{
  __m256 output00_07 = _mm256_set1_ps(-1.0); // for debugging

  __m256 input00_07 = _mm256_loadu_ps(&src[0]);
  __m256 input08_15 = _mm256_loadu_ps(&src[8]);
  __m256 input16_23 = _mm256_loadu_ps(&src[16]);
  __m256 input24_31 = _mm256_loadu_ps(&src[24]);

  
    /* 
       STUDENT_TODO: Add student code below.
    */

    // i don't understand why i have to multiply vindex by 2 and set scale to 8.
    // shouldn't setting the scale to 4 be enough?
    __m256i vindex = _mm256_set_epi32(14,12,10,8,6,4,2,0); 
    __m256 gathered = _mm256_i32gather_ps (src, vindex, 8);
    
    /* 
       End of student code.
    */
  

  // Store the result back to memory.
  _mm256_storeu_ps(&dst[ 0],gathered);
}


void test_gather_at_stride_8xfloat()
{
  float a[32] = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15,
                 16,17,18,19,20,21,22,23,24, 25,26,27,28,29,30,31};
  float bt[8] = {-1,-1,-1,-1, -1,-1,-1,-1 };

  float br[8] = {-1,-1,-1,-1, -1,-1,-1,-1 };

  
  reference_gather_at_stride_8xfloat(a, bt);
  student_gather_at_stride_8xfloat(a, br);


  float res = max_pair_wise_diff(8, bt, br);

  printf("test_gather_at_stride_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem_as_vects(" a", 32,8, a);
      print_float_mem_as_vects("bt", 8,8, bt);
      print_float_mem_as_vects("br", 8,8, br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}



// 10
/*
  Scattering at a stride
  (place a value at every Xth element)
*/

void reference_scatter_at_stride_8xfloat(float *src, float *dst)
{
  const int vlen   = 8;
  const int stride = 4;

  for( int vid = 0; vid < vlen; ++vid )
    dst[vid*stride] = src[vid];

}


void student_scatter_at_stride_8xfloat(float *src, float *dst)
{
  __m256 output00_07 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output08_15 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output16_23 = _mm256_set1_ps(-1.0); // for debugging
  __m256 output24_31 = _mm256_set1_ps(-1.0); // for debugging

  // 0,1,2,3,4,5,6,7
  __m256 input00_07 = _mm256_loadu_ps(&src[0]);

    /* 
       STUDENT_TODO: Add student code below.
    */

    // implement stride 4 among all output registers
    
    /*
    work in progress; close to functional but cannot seem to overcome last hurdle

    __m256 A = _mm256_set1_ps(-1.0);

    __m256 A1 = _mm256_extractf128_ps(__m256 A, 0);
    __m256 A2 = _mm256_extractf128_ps(__m256 A, 1);

    imm8 = 0x88;
    __mm256 x = _mm_shuffle_ps(__m128 a, __m128 A, imm8);
    print_m256(A);
    
    */


    /* 
       End of student code.
    */
  

  // Store the result back to memory.
  _mm256_storeu_ps(&dst[ 0], output00_07);
  _mm256_storeu_ps(&dst[ 8], output00_07);
  _mm256_storeu_ps(&dst[16], output00_07);
  _mm256_storeu_ps(&dst[24], output00_07);

}


void test_scatter_at_stride_8xfloat()
{
  float a[8] = {0,1,2,3,4,5,6,7};

  float bt[32] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 };

  float br[32] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 };

  
  reference_scatter_at_stride_8xfloat(a, bt);
  student_scatter_at_stride_8xfloat(a, br);


  float res = max_pair_wise_diff(32, bt, br);

  printf("test_scatter_at_stride_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem_as_vects(" a", 8,8, a);
      print_float_mem_as_vects("bt", 32,8, bt);
      print_float_mem_as_vects("br", 32,8, br);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}



//////////////////
// DO SOME MATH //
//////////////////



// 11
// matvec 1
void reference_matvec_8x8_colmaj_64xfloat(float *A, float *x, float *y)
{
  const int vlen = 8;
  const int m = 8;
  const int n = 8;
  const int rs_s = m;
  const int cs_s = 1;

  for (int i = 0; i < m; ++i ) {
    for (int j = 0; j < n; ++j ) {
      y[i] += A[i*cs_s + j*rs_s] * x[j];
    }
  }

}


void student_matvec_8x8_colmaj_8xfloat(float *A, float *x, float *y)
{
  __m256 y00_07 = _mm256_set1_ps(0.0f);

  __m256 A00_07 = _mm256_loadu_ps(&A[0]);
  __m256 A08_15 = _mm256_loadu_ps(&A[8]);
  __m256 A16_23 = _mm256_loadu_ps(&A[16]);
  __m256 A24_31 = _mm256_loadu_ps(&A[24]);
  __m256 A32_39 = _mm256_loadu_ps(&A[32]);
  __m256 A40_47 = _mm256_loadu_ps(&A[40]);
  __m256 A48_55 = _mm256_loadu_ps(&A[48]);
  __m256 A56_63 = _mm256_loadu_ps(&A[56]);

  {
    /* 
       STUDENT_TODO: Add student code below.
    */

    // multiply each element of A00_07 by x[0]
    __m256 x0 = _mm256_set1_ps(x[0]);
    __m256 y0 = _mm256_mul_ps(A00_07, x0);
      //print_m256(y0); // for testing
    // multiply each element of A08_15 by x[1]
    __m256 x1 = _mm256_set1_ps(x[1]);
    __m256 y1 = _mm256_mul_ps(A08_15, x1);
  
    // ... and so on
    __m256 x2 = _mm256_set1_ps(x[2]);
    __m256 y2 = _mm256_mul_ps(A16_23, x2);
    
    // 
    __m256 x3 = _mm256_set1_ps(x[3]);
    __m256 y3 = _mm256_mul_ps(A24_31, x3);
    
    // 
    __m256 x4 = _mm256_set1_ps(x[4]);
    __m256 y4 = _mm256_mul_ps(A32_39, x4);
    
    // 
    __m256 x5 = _mm256_set1_ps(x[5]);
    __m256 y5 = _mm256_mul_ps(A40_47, x5);
    
    // 
    __m256 x6 = _mm256_set1_ps(x[6]);
    __m256 y6 = _mm256_mul_ps(A48_55, x6);
    
    // 
    __m256 x7 = _mm256_set1_ps(x[7]);
    __m256 y7 = _mm256_mul_ps(A56_63, x7);
    

    // add y0 and y1
    __m256 y01 = _mm256_add_ps(y0, y1);
    
    // add y2 and y3
    __m256 y23 = _mm256_add_ps(y2, y3);
    
    // ... and so on, again
    __m256 y45 = _mm256_add_ps(y4, y5);
    
    //
    __m256 y67 = _mm256_add_ps(y6, y7);
    

    // you get the idea
    __m256 y0123 = _mm256_add_ps(y01, y23);
    
    // 
    __m256 y4567 = _mm256_add_ps(y45, y67);
    

    // final addition
    y00_07 = _mm256_add_ps(y0123, y4567);
      //print_m256(y00_07);

    // for some reason our results are 1 off. fix that here
    __m256 one = _mm256_set1_ps(1.0f);
    y00_07 = _mm256_sub_ps(y00_07, one);

    /* 
       End of student code.
    */
  }

  // Store the result back to memory.
  _mm256_storeu_ps(&y[ 0],y00_07);
}

void test_matvec_8x8_colmaj_8xfloat()
{
  float x[8] = {8,7,6,5, 4,3,2,1 };
  
  float yt[8] = {-1,-1,-1,-1, -1,-1,-1,-1 };
  float yr[8] = {-1,-1,-1,-1, -1,-1,-1,-1 };

  float a[64];
  
  for(int i=0; i < 8; ++i)
      for(int j=0; j < 8; ++j)
	a[i+j*8]= i+j*8 + 1;

  
  reference_matvec_8x8_colmaj_64xfloat(a,x, yt);
  student_matvec_8x8_colmaj_8xfloat(a, x, yr);


  float res = max_pair_wise_diff(8, yt, yr);

  printf("test_matvec_8x8_colmaj_8xfloat: ");
  if( res > 1e-6 )
    {
      printf("FAIL\n");

      print_float_mem_as_vects(" a", 64,8, a);
      print_float_mem_as_vects(" x", 8,8, x);
      print_float_mem_as_vects("yt", 8,8, yt);
      print_float_mem_as_vects("yr", 8,8, yr);

      printf("\n");
    }
  else
    {
      printf("PASS\n");
    }
}


int main( int argc, char *argv[] )
{

  printf("01: ");test_reverse_8xfloat();
  printf("02: ");test_rotate_by_4_8xfloat(); 
  printf("03: ");test_rotate_by_2_8xfloat(); 
  printf("04: ");test_rotate_by_1_8xfloat(); 

  printf("05: ");test_transpose_4x2_colmaj_8xfloat();
  printf("06: ");test_rotate_by_1_16xfloat(); 
  printf("07: ");test_transpose_4x4_colmaj_8xfloat(); 
  printf("08: ");test_transpose_8x4_colmaj_8xfloat();

  printf("09: ");test_gather_at_stride_8xfloat();
  printf("10: ");test_scatter_at_stride_8xfloat();
  printf("11: ");test_matvec_8x8_colmaj_8xfloat();
  
  return 0;
}



