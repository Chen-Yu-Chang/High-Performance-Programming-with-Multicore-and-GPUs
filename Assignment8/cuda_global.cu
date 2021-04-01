#include <math.h>
#include <cstdio>
#include <cstdlib>
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

#define GIG 1000000000
#define NANO_TO_MILLI 1000000
#define CPG 2.53         // Cycles per GHz -- Adjust to your computer

#define NUM_THREADS_PER_BLOCK 	16
#define NUM_BLOCKS 				16
#define PRINT_TIME 				1
#define SM_ARR_LEN				50000
#define TOL						1e-6

#define NN 2048		//matrix size

#define THREADS_PER_BLOCK 16

#define THREADS_PER_BLOCK_Y 16
#define NUM_BLOCKS_X 16
#define NUM_BLOCKS_Y 16


#define IMUL(a, b) __mul24(a, b)



void initialize_array(float *A, int len, int seed);
void write_2d_array_to_file(float *A, char *filename);
struct timespec diff(struct timespec start, struct timespec end);

__global__ void kernel_MMM(float *A, float *B, float *C, int N){
	int j = (blockIdx.x * blockDim.x) + threadIdx.x;
	int i = (blockIdx.y * blockDim.y) + threadIdx.y;
	int k;
	float sum = 0;
	if(i > N || j > N)
		return;

	for(k = 0; k < N; k++){
		sum += (A[i*N+k] * B[k*N+j]);
	}
	C[i*N+j] = sum;
}

int main(int argc, char **argv){
	// GPU Timing variables
	int i, j, k;
	int not_a_number;

	cudaEvent_t start_i, start_o, stop_i, stop_o;
	float elapsed_gpu_internal, elapsed_gpu_with_copy;
	float max_difference, min_difference, average_difference, difference, average;
	struct timespec diff(struct timespec start, struct timespec end);
	struct timespec time1, time2, elapsed_cpu;


	//Arrays on GPU global memory
	float *g_A, *g_B, *g_C;

	//arrays on host memory
	float *h_A, *h_B, *h_C, *h_C_control;

	//Allocate arrays on GPU memory
	printf("Allocating arrays\n");
	CUDA_SAFE_CALL(cudaMalloc((void **) &g_A, NN*NN*sizeof(float)));
	CUDA_SAFE_CALL(cudaMalloc((void **) &g_B, NN*NN*sizeof(float)));
	CUDA_SAFE_CALL(cudaMalloc((void **) &g_C, NN*NN*sizeof(float)));

	//Allocate arrays on host memory
	h_A = (float*) malloc(NN*NN*sizeof(float));
	h_B = (float*) malloc(NN*NN*sizeof(float));
	h_C = (float*) malloc(NN*NN*sizeof(float));
	h_C_control = (float*) malloc(NN*NN*sizeof(float));

	//initialize host arrays
	printf("Initializing host arrays\n");
	initialize_array(h_A, NN*NN, 1000);
	initialize_array(h_B, NN*NN, 2000);

	//create cuda events
	printf("Starting outer cuda timing\n");
	cudaEventCreate(&start_o);
	cudaEventCreate(&stop_o);
	//record event on default stream
	cudaEventRecord(start_o, 0);

	//transfer array to GPU memory
	printf("Transfering arrays to GPU\n");
	CUDA_SAFE_CALL(cudaMemcpy(g_A, h_A, NN*NN*sizeof(float), cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(g_B, h_B, NN*NN*sizeof(float), cudaMemcpyHostToDevice));

	//Launch the kernel
	//NN*NN/256 = # of blocks

	dim3 dimGrid((NN+THREADS_PER_BLOCK-1)/THREADS_PER_BLOCK,(NN+THREADS_PER_BLOCK-1)/THREADS_PER_BLOCK);
	dim3 dimBlock(THREADS_PER_BLOCK,THREADS_PER_BLOCK);

	//launch the kernel
	printf("Starting outer cuda timing\n");
	cudaEventCreate(&start_i);
	cudaEventCreate(&stop_i);
	//record event on default stream
	cudaEventRecord(start_i, 0);

	printf("Running kernel\n");
	kernel_MMM<<<dimGrid, dimBlock>>>(g_A, g_B, g_C, NN);
	cudaThreadSynchronize();

	cudaEventRecord(stop_i,0);
	cudaEventSynchronize(stop_i);
	cudaEventElapsedTime(&elapsed_gpu_internal, start_i, stop_i);
	cudaEventDestroy(start_i);
	cudaEventDestroy(stop_i);

	//check for errors during launch
	CUDA_SAFE_CALL(cudaPeekAtLastError());

	//transfer results back to host
	printf("Copying cuda results to host\n");
	CUDA_SAFE_CALL(cudaMemcpy(h_C, g_C, NN*NN*sizeof(float), cudaMemcpyDeviceToHost));

	//stop and destroy the timer
	cudaEventRecord(stop_o,0);
	cudaEventSynchronize(stop_o);
	cudaEventElapsedTime(&elapsed_gpu_with_copy, start_o, stop_o);
	cudaEventDestroy(start_o);
	cudaEventDestroy(stop_o);


	//compute result on host
	printf("Computing MMM on host\n");
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

	for(i = 0; i < NN; i++){
		for(j = 0; j < NN; j++){
			h_C_control[i*NN+j] = 0;
			for(k = 0; k < NN; k++){
				h_C_control[i*NN+j] += (h_A[i*NN+k] * h_B[k*NN + j]);
			}
		}
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
	elapsed_cpu = diff(time1, time2);

	//write arrays to file
	
	//printf("Writing arrays to file\n");
	//write_2d_array_to_file(h_A, "A_matrix.txt");
	//write_2d_array_to_file(h_C, "gpu_MMM.txt");
	//write_2d_array_to_file(h_C_control, "cpu_MMM.txt");
	

	//compare the results
	printf("Comparing results\n");
	max_difference = 0;
	min_difference = NN*NN;
	average_difference = 0;
	average = 0;
	not_a_number = 0;

	for(i = 0; i < NN; i++){
		for(j = 0; j < NN; j++){
			if(h_C[i*NN+j] != h_C[i*NN+j])
				not_a_number++;
			else{
				average += h_C[i*NN+j];
				average += h_C_control[i*NN+j];
				difference = abs(h_C[i*NN+j] - h_C_control[i*NN+j]);
				if(difference > max_difference)
					max_difference = difference;
				if(difference < min_difference)
					min_difference = difference;
				average_difference += difference;
			}
		}
	}
	average_difference /= (float)(NN*NN);
	average /= (float) (2*NN*NN);

	//free memory
	printf("Freeing memory\n");
	CUDA_SAFE_CALL(cudaFree(g_A));
	CUDA_SAFE_CALL(cudaFree(g_B));
	CUDA_SAFE_CALL(cudaFree(g_C));
	free(h_A);
	free(h_B);
	free(h_C);
	free(h_C_control);

	//print results
	printf("Printing results\n");
	printf("\nGPU outer loop time: %f (msec)\n", elapsed_gpu_with_copy);
	printf("\nGPU inner loop time: %f (msec)\n", elapsed_gpu_internal);
	printf("\nCPU time: %f(msec)\n", (float)(((double)GIG*elapsed_cpu.tv_sec + elapsed_cpu.tv_nsec)/(double)NANO_TO_MILLI));
	printf("Max difference = %f, Min difference = %f, Average difference = %f, Average = %f, Not a number = %d\n", max_difference, min_difference, average_difference, average, not_a_number);
	printf("Max Tolerance = %f%%, Min Tolerance = %f%%, Average Tolerance = %f%% NaN Tolerance = %f%%\n", max_difference/average*100, min_difference/average*100, average_difference/average*100, (float)not_a_number/(float)(NN*NN)*(float)100);

	return 0;
}

void initialize_array(float *A, int len, int seed){
	int i;
	float randNum;

	srand(seed);

	for(i = 0; i < len; i++){
		randNum = (float) rand();
		A[i] = randNum;
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

void write_2d_array_to_file(float *A, char *filename){
	int i;
	FILE *f = fopen(filename, "w");
	for(i = 0; i < NN*NN; i++){
		fprintf(f, "%.0f, ", A[i]);
		if(i%NN == 0)
			fprintf(f, "\n");
	}
	fclose(f);
}
