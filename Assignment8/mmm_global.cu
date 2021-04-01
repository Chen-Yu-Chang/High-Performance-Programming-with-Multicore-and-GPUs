#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <math.h>

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
#define SM_ARR_LEN				1024
#define NUM_BLOCKS 				(SM_ARR_LEN*SM_ARR_LEN + NUM_THREADS_PER_BLOCK-1)/NUM_THREADS_PER_BLOCK
#define TOL						1//0.00001
#define OMEGA 1.60

#define IMUL(a, b) __mul24(a, b)

void initializeArray1D(float *arr, int len, int seed);

__global__ void kernel_mmm (float* A, float* B, float* C, int N) {

  int row = blockIdx.y*blockDim.y+threadIdx.y; 
  int col = blockIdx.x*blockDim.x+threadIdx.x;

  float tmpSum = 0;

  if(row < N && col<N){
	
	for(int i = 0; i<N; i++){
		tmpSum += A[row * N + i] * B[i * N + col];
	}

   }
   C[row * N + col] = tmpSum;
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
	initializeArray1D(h_y, arrLen, 1467);
	//initializeArray1D(h_result, arrLen, 2453);
	//initializeArray1D(h_result_gold, arrLen, 1467);
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
	CUDA_SAFE_CALL(cudaMemcpy(d_x, h_x, allocSize, cudaMemcpyHostToDevice));
	  
	// Launch the kernel
	dim3 dimBlock(16,16);
	kernel_mmm<<<NUM_BLOCKS, dimBlock>>>(d_x, d_y, d_result, arrLen);

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


clock_t begin = clock();

	// Compute the results on the host
   int l, m, n;
   int length = arrLen;
   float sum;

  for (l = 0; l < length; l++) {
    for (m = 0; m < length; m++) {
       sum = 0;
      for (n = 0; n < length; n++){
        sum += h_x[l*length+n] * h_y[n*length+m];
	}
      h_result_gold[l*length+m] += sum;
    }
  }

clock_t end = clock();

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
	
	for(i = 0; i < 50; i++) {
		printf("%d:\t%.8f\t%.8f\n", i, h_result_gold[i], h_result[i]);
	}
	
	double time_spent;

	time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Time elpased is %f seconds", time_spent);
	
	/*if (errCount > 0) {
		printf("\n@ERROR: TEST FAILED: %d results did not matched\n", errCount);
	}
	else if (zeroCount > 0){
		printf("\n@ERROR: TEST FAILED: %d results (from GPU) are zero\n", zeroCount);
	}
	else {
		printf("\nTEST PASSED: All results matched\n");
	}*/
	
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
		randNum =  randNum/RAND_MAX;
		arr[i*len + j] = randNum;
	    }
	}
}





