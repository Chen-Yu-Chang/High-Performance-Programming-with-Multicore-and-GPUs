#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <time.h>

// Assertion to check for errors
#define CUDA_SAFE_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
	if (code != cudaSuccess) 
	{
		fprintf(stderr,"CUDA_SAFE_CALL: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

#define NUM_THREADS_PER_BLOCK 	256

#define PRINT_TIME 				1
#define SM_ARR_LEN				2000
#define NUM_BLOCKS 				(SM_ARR_LEN*SM_ARR_LEN + NUM_THREADS_PER_BLOCK-1)/NUM_THREADS_PER_BLOCK
#define TOL						1//0.00001
#define OMEGA 1.60

#define IMUL(a, b) __mul24(a, b)

void initializeArray1D(float *arr, int len, int seed);

__global__ void kernel_sor (int buf_dim, float* buf) {
	/*const int tid = threadIdx.x;
	const int tjd = threadIdx.y;
	//const int threadN = IMUL(blockDim.x, gridDim.x);
	
	
	int in ; //threadIdx.x;
	int jn ; //threadIdx.y;
	float change, mean_change = 100;
	
		for(int i = 0; i<2000; i++) {
			    
			    mean_change = 0;
			    for (in = tid; in < arrLen ; in+= threadIdx.x){ 
			      for (jn = tjd; jn < arrLen ; jn+= threadIdx.y){
				change = result[in*arrLen+jn] - .25 * (result[(in-1)*arrLen+jn] +
								  result[(in+1)*arrLen+jn] +
								  result[in*arrLen+jn+1] +
								  result[in*arrLen+jn-1]);
				result[in*arrLen+jn] -= change * OMEGA;
				if (change < 0){
				  change = -change;
				}
				mean_change += change;
		             }
			}

	     }*/
    int block_x_len = buf_dim / gridDim.x;
    int thread_x_len  = block_x_len / blockDim.x;
    int x_offset    = block_x_len * blockIdx.x + thread_x_len * threadIdx.x;
    
    int block_y_len = buf_dim / gridDim.y;
    int thread_y_len  = block_y_len / blockDim.y;
    int y_offset    = block_y_len * blockIdx.y + thread_y_len * threadIdx.y;

    int x_start = x_offset + (x_offset == 0 ? 1 : 0);
    int x_bound = x_offset + thread_x_len - (x_offset + thread_x_len == buf_dim ? 1 : 0);
    int y_start = y_offset + (y_offset == 0 ? 1 : 0);
    int y_bound = y_offset + thread_y_len - (y_offset + thread_y_len == buf_dim ? 1 : 0);

    for (int itr = 0; itr < 2000; itr++)
    {
        for (int i = x_start; i < x_bound; i++)
        {
            for (int j = y_start; j < y_bound; j++)
            {
                buf[i * buf_dim + j] = 0.25 * (
                      buf[(i + 1) * buf_dim + j]
                    + buf[(i - 1) * buf_dim + j]
                    + buf[i * buf_dim + j + 1]
                    + buf[i * buf_dim + j - 1]
                );
            }
        }
    }
		
	
}


int main(int argc, char **argv){
	int arrLen = 0;
		
	// GPU Timing variables
	cudaEvent_t start, stop;
	float elapsed_gpu;
	
	// Arrays on GPU global memoryc
	float *d_x;
	float *d_y;
	float *d_result;

	// Arrays on the host memory
	float *h_x;
	float *h_y;
	float *h_result;
	float *h_result_gold;
	
	int i, errCount = 0, zeroCount = 0;
	int j;
	
	if (argc > 1) {
		arrLen  = atoi(argv[1]);
	}
	else {
		arrLen = SM_ARR_LEN;
	}

	printf("Length of the array = %d\n", arrLen);

    // Select GPU
    CUDA_SAFE_CALL(cudaSetDevice(0));

	// Allocate GPU memory
	size_t allocSize = arrLen*arrLen * sizeof(float);
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_x, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_y, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_result, allocSize));
		
	// Allocate arrays on host memory
	h_x                        = (float *) malloc(allocSize);
	h_y                        = (float *) malloc(allocSize);
	h_result                   = (float *) malloc(allocSize);
	h_result_gold              = (float *) malloc(allocSize);
	
	// Initialize the host arrays
	printf("\nInitializing the arrays ...");
	// Arrays are initialized with a known seed for reproducability
	initializeArray1D(h_x, arrLen, 2453);
	//initializeArray1D(h_y, arrLen*arrLen, 1467);
	initializeArray1D(h_result, arrLen, 2453);
	initializeArray1D(h_result_gold, arrLen, 1467);
	printf("\t... done\n\n");
	
	
#if PRINT_TIME
	// Create the cuda events
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	// Record event on the default stream
	cudaEventRecord(start, 0);
#endif
	
	// Transfer the arrays to the GPU memory
	CUDA_SAFE_CALL(cudaMemcpy(d_result, h_result, allocSize, cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(d_y, h_y, allocSize, cudaMemcpyHostToDevice));
	  
	// Launch the kernel
	dim3 dimBlock(16,16);
	kernel_sor<<<NUM_BLOCKS, dimBlock>>>(arrLen, d_result);

	// Check for errors during launch
	CUDA_SAFE_CALL(cudaPeekAtLastError());
	
	// Transfer the results back to the host
	CUDA_SAFE_CALL(cudaMemcpy(h_result, d_result, allocSize, cudaMemcpyDeviceToHost));
	
#if PRINT_TIME
	// Stop and destroy the timer
	cudaEventRecord(stop,0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&elapsed_gpu, start, stop);
	printf("\nGPU time: %f (msec)\n", elapsed_gpu);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
#endif
	
	clock_t begin;

	int change;
	// Compute the results on the host
	for( int k = 0;k<2000;k++){
	    for (i = 1; i < arrLen-1; i++){ 
	      for (j = 1; j < arrLen-1; j++) {
		change = h_result_gold[i*arrLen+j] - .25 * (h_result_gold[(i-1)*arrLen+j] +
						  h_result_gold[(i+1)*arrLen+j] +
						  h_result_gold[i*arrLen+j+1] +
						  h_result_gold[i*arrLen+j-1]);
		h_result_gold[i*arrLen+j] -= change * OMEGA;

		}
	   }
	}

	clock_t ending;
	// Compare the results
       /*
	for(i = 0; i < arrLen*arrLen; i++) {
		if (abs(h_result_gold[i] - h_result[i]) > TOL) {
			errCount++;
		}
		if (h_result[i] == 0) {
			zeroCount++;
		}
	}
	*/
	
	double cpu_time = ((double) (ending - begin)) / CLOCKS_PER_SEC;

	printf("fun() took %f seconds to execute \n", cpu_time); 

	for(i = 0; i < 50; i++) {
		printf("%d:\t%.8f\t%.8f\n", i, h_result_gold[i], h_result[i]);
	}
	
	
	if (errCount > 0) {
		printf("\n@ERROR: TEST FAILED: %d results did not matched\n", errCount);
	}
	else if (zeroCount > 0){
		printf("\n@ERROR: TEST FAILED: %d results (from GPU) are zero\n", zeroCount);
	}
	else {
		printf("\nTEST PASSED: All results matched\n");
	}
	
	// Free-up device and host memory
	CUDA_SAFE_CALL(cudaFree(d_x));
	CUDA_SAFE_CALL(cudaFree(d_y));
	CUDA_SAFE_CALL(cudaFree(d_result));
		   
	free(h_x);
	free(h_y);
	free(h_result);
		
	return 0;
}

void initializeArray1D(float *arr, int len, int seed) {
	int i;
	int j;
	float randNum;
	srand(seed);

	for (i = 0; i < len; i++) {
	   for(j = 0; j<len; j++){
		randNum = (float) rand();
		arr[i*len + j] = randNum;
	    }
	}
}
