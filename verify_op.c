#include <assert.h>
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ERROR_THRESHOLD 1e-4

extern void COMPUTE_NAME_REF( int m0,
			      float *input_distributed,
			      float *output_distributed );

extern void COMPUTE_NAME_TST( int m0,
			      float *input_distributed,
			      float *output_distributed );


extern void DISTRIBUTED_ALLOCATE_NAME_REF( int m0,
					   float **input_distributed,
					   float **output_distributed );

extern void DISTRIBUTED_ALLOCATE_NAME_TST( int m0,
					   float **input_distributed,
					   float **output_distributed );



extern void DISTRIBUTE_DATA_NAME_REF( int m0,
				      float *input_sequential,
				      float *input_distributed);

extern void DISTRIBUTE_DATA_NAME_TST( int m0,
				      float *input_sequential,
				      float *input_distributed);




extern void COLLECT_DATA_NAME_REF( int m0,
				   float *output_distributed,
				   float *output_sequential );

extern void COLLECT_DATA_NAME_TST( int m0,
				   float *output_distributed,
				   float *output_sequential );




extern void DISTRIBUTED_FREE_NAME_REF( int m0,
				       float *input_distributed,
				       float *output_distributed );

extern void DISTRIBUTED_FREE_NAME_TST( int m0,
				       float *input_distributed,
				       float *output_distributed );




void fill_buffer_with_random( int num_elems, float *buff )
{
  long long range = RAND_MAX;
  //long long range = 1000;
  
  for(int i = 0; i < num_elems; ++i)
    {
      buff[i] = ((float)(rand()-((range)/2)))/((float)range);
    }
}

void fill_buffer_with_value( int num_elems, float val, float *buff )
{
  for(int i = 0; i < num_elems; ++i)
    buff[i] = val;
}


float max_pair_wise_diff(int m, int n, int rs, int cs, float *a, float *b)
{
  float max_diff = 0.0;

  for(int i = 0; i < m; ++i)
    for(int j = 0; j < n; ++j)
      {
	float sum  = fabs(a[i*rs+j*cs]+b[i*rs+j*cs]);
	float diff = fabs(a[i*rs+j*cs]-b[i*rs+j*cs]);

	float res = 0.0f;

	if(sum == 0.0f)
	  res = diff;
	else
	  res = 2*diff/sum;

	if( res > max_diff )
	  max_diff = res;
      }

  return max_diff;
}


int scale_p_on_pos_ret_v_on_neg(int p, int v)
{
  if (v < 1)
    return -1*v;
  else
    return v*p;
}

int main( int argc, char *argv[] )
{
  int rid;
  int num_ranks;
  int tag = 0;
  MPI_Status  status;
  int root_rid = 0;

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rid);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

  // What we will output to
  FILE *result_file;
  
  // Problem parameters
  int min_size;
  int max_size;
  int step_size;

  int in_m0;

  // Get command line arguments
  if(argc == 4 + 1 || argc == 5 + 1 )
    {
      min_size  = atoi(argv[1]);
      max_size  = atoi(argv[2]);
      step_size = atoi(argv[3]);

      in_m0=atoi(argv[4]);

      /*
      printf("min_size: %d, max_size: %d, step_size: %d, in_m0: %d, in_max_offset: %d\n",
              min_size, max_size, step_size, in_m0, in_max_offset);
      */

      // default to printing to stdout
      result_file = stdout;

      if(argc == 5 + 1)
	{
	  // we don't want every node opening the same file
	  // to write to.
	  if(rid == 0 )
	    {
	      result_file = fopen(argv[5],"w");
	    }
	  else
	    {
	      result_file = NULL;
	    }
	}
    }
  else
    {
      printf("usage: %s min max step m0 [filename]\n",
	     argv[0]);
      exit(1);
    }

  // Print out the first line of the output in csv format
  if( rid == 0 )
    {
      /*root node */ 
      fprintf(result_file, "num_ranks,m0,result\n");
    }
  else
    {/* all other nodes*/ }


  for( int p = min_size;
       p < max_size;
       p += step_size )
    {

      // input sizes
      int m0=scale_p_on_pos_ret_v_on_neg(p,in_m0);

      // How big of a buffer do we need
      int input_sequential_sz  =m0;
      int output_sequential_sz =m0;

      float *input_sequential_ref   = (float *)calloc((input_sequential_sz), sizeof(float));
      float *output_sequential_ref  = (float *)calloc(output_sequential_sz, sizeof(float));

      float *input_sequential_tst   = (float *)calloc((input_sequential_sz), sizeof(float));
      float *output_sequential_tst  = (float *)calloc(output_sequential_sz, sizeof(float));

      assert(NULL != input_sequential_ref);
      assert(NULL != output_sequential_ref);
      assert(NULL != input_sequential_tst);
      assert(NULL != output_sequential_tst);


      // We only want to allocate the buffers on every node, but
      // we don't want to fill them with random data on every node
      // just from the root node.

      if( rid == 0)
	{ /* root node */

	  // fill src_ref with random values
	  fill_buffer_with_random( input_sequential_sz, input_sequential_ref );
	  fill_buffer_with_value( output_sequential_sz, -1, output_sequential_ref );
     
	  // copy src_ref to src_tst
	  memcpy(input_sequential_tst,input_sequential_ref,input_sequential_sz*sizeof(float));
	  memcpy(output_sequential_tst,output_sequential_ref,output_sequential_sz*sizeof(float));
	}
      else
	{/* all other nodes. */}

      /*
	Run the reference
      */

      float *input_distributed_ref;
      float *output_distributed_ref;

      // Allocate distributed buffers for the reference
      DISTRIBUTED_ALLOCATE_NAME_REF( m0,
				     &input_distributed_ref,
				     &output_distributed_ref );

      // Distribute the sequential buffers 
      DISTRIBUTE_DATA_NAME_REF( m0,
				input_sequential_ref,
				input_distributed_ref );
     
      // Perform the computation
      COMPUTE_NAME_REF( m0,
			input_distributed_ref,
			output_distributed_ref );


      // Collect the distributed data and write it to a sequential buffer
      COLLECT_DATA_NAME_REF( m0,
			     output_distributed_ref,
			     output_sequential_ref );     
     
      // Finally free the buffers
      DISTRIBUTED_FREE_NAME_REF( m0,
				 input_distributed_ref,
				 output_distributed_ref );
     

      // run the test
      float *input_distributed_tst;
      float *output_distributed_tst;

      // Allocate distributed buffers for the reference
      DISTRIBUTED_ALLOCATE_NAME_TST( m0,
				     &input_distributed_tst,
				     &output_distributed_tst );

      // Distribute the sequential buffers 
      DISTRIBUTE_DATA_NAME_TST( m0,
				input_sequential_tst,
				input_distributed_tst );
     
      // Perform the computation
      COMPUTE_NAME_TST( m0,
			input_distributed_tst,
			output_distributed_tst );


      // Collect the distributed data and write it to a sequential buffer
      COLLECT_DATA_NAME_TST( m0,
			     output_distributed_tst,
			     output_sequential_tst );     
     
      // Finally free the buffers
      DISTRIBUTED_FREE_NAME_TST( m0,
				 input_distributed_tst,
				 output_distributed_tst );


      // We only need to verify the results sequentially
      if( rid == 0)
	{
	  /* root node */
	  
	  float res = max_pair_wise_diff(m0,1,1,1, output_sequential_ref, output_sequential_tst);
	  
	  fprintf(result_file, "%i,%i,",
		  num_ranks,
		  m0);
	  
	  // if our error is greater than some threshold
	  if( res > ERROR_THRESHOLD ) {
      fprintf(result_file, "FAIL\n");

      // fprintf(result_file, "ref: ");
      // for (int i = 0; i < m0; i++) {
      //   fprintf(result_file, "%f ", output_sequential_ref[i]);
      // }
      // fprintf(result_file, "\n");

      // fprintf(result_file, "tst: ");
      // for (int i = 0; i < m0; i++) {
      //   fprintf(result_file, "%f ", output_sequential_tst[i]);
      // }
      // fprintf(result_file, "\n");

      // fprintf(result_file, "error: %f\n", res);
    }
	  else
	    fprintf(result_file, "PASS\n");
	}
      else
	{/* all other nodes */}

      // Free the sequential buffers
      free(input_sequential_ref);
      free(output_sequential_ref);
      free(input_sequential_tst);
      free(output_sequential_tst);

    }


  // Only needs to be done by root node
  if(rid == 0)
    {
      /* root node */
      fclose(result_file);
    }
  else
    {/* all other nodes */}
     

  
 MPI_Finalize();
}
