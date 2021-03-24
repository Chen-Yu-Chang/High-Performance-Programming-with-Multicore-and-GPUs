#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>

#define GIG 1000000000
#define NANO_TO_MILLI 1000000
#define CPG 2.8         // Cycles per GHz -- Adjust to your computer

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

const int MATRIX_SIZE = 2000;
const int NUM_BLOCKS = 1;
const int THREADS_PER_BLOCK_X = 16;
const int THREADS_PER_BLOCK_Y = 16;
const int SOR_ITERATIONS = 2000;
const int OMEGA = 1;

#define ALLOCATE_AND_INIT
#define TRANSFER_TO_GPU
#define LAUNCH_KERNEL
#define TRANSFER_RESULTS
#define COMPUTE_CPU_RESULTS
#define COMPARE_RESULTS
#define FREE_MEMORY
#define GPU_TIMING
#define CPU_TIMING
#define DEBUG_PRINT
#define WRITE_2D_ARRAYS

void initialize_array_2D(float **A, int len, int seed);

__global__ void kernel_SOR_internal(float *A, int omega, int N_x, int N_y){
	int i, j = 0;
	//different divisions needed for group of threads etc.
	int xx = blockIdx.x * blockDim.x + threadIdx.x;
	int yy = blockIdx.x * blockDim.y + threadIdx.y;
	float phi;
	for(i = MATRIX_SIZE/THREADS_PER_BLOCK_X*xx; i < MATRIX_SIZE/THREADS_PER_BLOCK_X*(xx+1); i++){
		for(j = MATRIX_SIZE/THREADS_PER_BLOCK_Y*yy; j < MATRIX_SIZE/THREADS_PER_BLOCK_Y*(yy+1); j++){
			if(i > 0 && i < (N_x-1) && j > 0 && j < (N_y-1)){
				phi = A[i*MATRIX_SIZE + j] - .25*((A[(i-1)*MATRIX_SIZE + j] + A[(i+1)*MATRIX_SIZE+j]) + (A[i*MATRIX_SIZE + (j-1)] + A[i*MATRIX_SIZE+(j+1)]));
				A[i*MATRIX_SIZE+j] = abs(A[i*MATRIX_SIZE+j] - (phi*omega));
			}
		}
	}
}

void SOR_internal_sequential(float **A, int omega, int xx, int yy, int N_x, int N_y){
	float phi;
	if(xx > 0 && xx < N_x-1 && yy > 0 && yy < N_y-1){
		phi = A[xx][yy] - .25*((A[xx-1][yy] + A[xx+1][yy]) + (A[xx][yy-1] + A[xx][yy+1]));
		A[xx][yy] = abs(A[xx][yy] - (phi*omega));
	}
}

void write_2d_array_to_file(float **A, int N_x, int N_y, char *filename){
	int i, j;
	FILE *f = fopen(filename, "w");
	for(i = 0; i < N_x; i++){
		for(j = 0; j < N_y-1; j++){
			fprintf(f,"%f, ",A[i][j]);
		}
		fprintf(f, "%f\n", A[i][N_y-1]);
	}
	fclose(f);
}

int main(int argc, char **argv){

	//loop variables
	int i, j, k, errors = 0;

	//timing variables
	cudaEvent_t start, stop;
	float elapsed_gpu;
	struct timespec diff(struct timespec start, struct timespec end);
	struct timespec time1, time2, elapsed_cpu;

	//array dimensions
	dim3 dimGrid(NUM_BLOCKS,1,1);
	dim3 dimBlock(THREADS_PER_BLOCK_X,THREADS_PER_BLOCK_Y,1);
	
	//Arrays on GPU global memory
	float *g_A;

	//Arrays on host memory	
	float **h_A;
	float **h_A_test;

	//error file init
	FILE *f = fopen("mismatches.txt", "w");
	if(f == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

#ifdef DEBUG_PRINT
	printf("init done\n");
#endif

	//Allocate arrays on GPU memory
#ifdef ALLOCATE_AND_INIT
	CUDA_SAFE_CALL(cudaMalloc((void **) &g_A, MATRIX_SIZE*MATRIX_SIZE*sizeof(float)));

	//Allocate arrays on host memory
	h_A = (float**) malloc(MATRIX_SIZE * sizeof(float*));
	h_A_test = (float**) malloc(MATRIX_SIZE * sizeof(float*));
	for(i = 0; i < MATRIX_SIZE; i++){
		h_A[i] = (float*) malloc(MATRIX_SIZE * sizeof(float));
		h_A_test[i] = (float*) malloc(MATRIX_SIZE * sizeof(float));
	}

	//initialize host arrays
	initialize_array_2D(h_A, MATRIX_SIZE, 2453);
	initialize_array_2D(h_A_test, MATRIX_SIZE, 2453);
#endif 

#ifdef GPU_TIMING
	//create cuda events
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	//record event on default stream
	cudaEventRecord(start, 0);
#endif

#ifdef DEBUG_PRINT
	printf("all init done\n");
#endif
	//transfer array to GPU memory
#ifdef TRANSFER_TO_GPU
	for(i = 0; i < MATRIX_SIZE; i++){
		CUDA_SAFE_CALL(cudaMemcpy(&g_A[i*MATRIX_SIZE], h_A[i], MATRIX_SIZE, cudaMemcpyHostToDevice));
	}
#endif

	//launch the kernel
#ifdef LAUNCH_KERNEL
	for(i = 0; i < SOR_ITERATIONS; i++){
		kernel_SOR_internal<<<dimGrid, dimBlock>>>(g_A, OMEGA, MATRIX_SIZE, MATRIX_SIZE);
	}
#endif



	//check for errors during launch
	CUDA_SAFE_CALL(cudaPeekAtLastError());

#ifdef DEBUG_PRINT
	printf("kernel run\n");
#endif

	//transfer results back to host
#ifdef TRANSFER_RESULTS
	for(i = 0; i < MATRIX_SIZE; i++){
		CUDA_SAFE_CALL(cudaMemcpy(h_A[i], &g_A[i*MATRIX_SIZE], MATRIX_SIZE, cudaMemcpyDeviceToHost));
	}
#endif
	//stop and destroy the timer
#ifdef GPU_TIMING
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&elapsed_gpu, start, stop);
	printf("\nGPU time: %f (msec)\n", elapsed_gpu);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
#endif

#ifdef DEBUG_PRINT
	printf("results transfered\n");
#endif
	//compute results on host

#ifdef CPU_TIMING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
#endif

#ifdef COMPUTE_CPU_RESULTS
	for(i = 0; i < SOR_ITERATIONS; i++){
		for(j = 0; j < MATRIX_SIZE; j++){
			for(k = 0; k < MATRIX_SIZE; k++){
				SOR_internal_sequential(h_A_test, OMEGA, j, k, MATRIX_SIZE, MATRIX_SIZE);
			}
		}
	}
#endif

#ifdef CPU_TIMING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	elapsed_cpu = diff(time1,time2);
	printf("\nCPU time: %f(msec)\n", (float)(((double)GIG*elapsed_cpu.tv_sec + elapsed_cpu.tv_nsec)/(double)NANO_TO_MILLI));
#endif


#ifdef DEBUG_PRINT
	printf("results computed on CPU\n");
#endif
	//compare results

#ifdef COMPARE_RESULTS	
	for(i = 0; i < MATRIX_SIZE; i++){
		for(j = 0; j < MATRIX_SIZE; j++){
			if(h_A[i][j] != h_A_test[i][j]){
				errors++;
				fprintf(f, "Mismatch at [%d,%d] GPU = %f CPU = %f\n", i, j, h_A[i][j], h_A_test[i][j]);
			}
		}
	}
#endif

#ifdef DEBUG_PRINT
	printf("results checked\n");
#endif

#ifdef WRITE_2D_ARRAYS
	write_2d_array_to_file(h_A, MATRIX_SIZE, MATRIX_SIZE, "GPU_output.txt");
	write_2d_array_to_file(h_A_test, MATRIX_SIZE, MATRIX_SIZE, "CPU_output.txt");
#endif

	//errors
	printf("Found %d errors\n", errors);
	fclose(f);

	//free up memory
#ifdef FREE_MEMORY
	CUDA_SAFE_CALL(cudaFree(g_A));
	for(i = 0; i < MATRIX_SIZE; i++){
		free(h_A[i]);
		free(h_A_test[i]);
	}
	free(h_A);
	free(h_A_test);
#ifdef DEBUG_PRINT
	printf("arrays freed\n");
#endif
#endif

	return (float)0;
}

void initialize_array_2D(float **A, int len, int seed){
	int i, j;
	float randNum;
	srand(seed);

	for(i = 0; i < len; i++){
		for(j = 0; j < len; j++){
			randNum = (float) rand();
			A[i][j] = randNum;
		}
	}
}

struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}
