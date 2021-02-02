/*****************************************************************************/
// gcc -O1 -o test_mmm_block test_mmm_block.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define ITERS 1
#define DELTA 200
#define BASE  DELTA
#define BSIZE DELTA

#define MLENGTH 2400
#define BITERS 8
#define BBASE  5

#define OPTIONS 1
#define IDENT 0.0



/*****************************************************************************/
main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][BITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  int init_matrix(float* m, long int len);
  int zero_matrix(float* m, long int len);
  void mmm_ijk_blocked(float* a0, float* b0, float* c0, long int block_size, long int length);

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;
  long int bsize = BBASE;

  printf("\n Hello World -- MMM \n");

  // declare and initialize the matrix structure
  float* a0 = (float*) malloc(MLENGTH * MLENGTH * sizeof(float));
  init_matrix(a0, MLENGTH);
  float* b0 = (float*) malloc(MLENGTH * MLENGTH * sizeof(float));
  init_matrix(b0, MLENGTH);
  float* c0 = (float*) malloc(MLENGTH * MLENGTH * sizeof(float));
  zero_matrix(c0, MLENGTH);

  OPTION = 0;

  for (i = 0; i < BITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    mmm_ijk_blocked(a0,b0,c0,bsize,MLENGTH);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    printf("\niter = %d bsize = %d", i, bsize);
    bsize *= 2;
  }

  bsize = BBASE;
  for (i = 0; i < BITERS; i++) {
    printf("\n%d, %ld", bsize, (long int)((double)(CPG)*(double)
					  (GIG * time_stamp[0][i].tv_sec + time_stamp[0][i].tv_nsec)));
    bsize *= 2;
  }

  printf("\n");
  return 0;
}/* end main */

/**********************************************/


/* initialize matrix */
int init_matrix(float* m, long int len)
{
  long int i;

  if (len > 0) {
    for (i = 0; i < len*len; i++)
      m[i] = (float)(i);
    return 1;
  }
  else return 0;
}

/* initialize matrix */
int zero_matrix(float* m, long int len)
{
  long int i,j;

  if (len > 0) {
    for (i = 0; i < len*len; i++)
      m[i] = (float)(0);
    return 1;
  }
  else return 0;
}


/*************************************************/

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

/*************************************************/

/* mmm */
void mmm_ijk_blocked(float* a0, float* b0, float* c0, long int block_size, long int length)
{
  long int i, j, k, kk, jj;
  float sum;

  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++) c0[i*length+j] = 0.0;

  for (kk = 0; kk < length; kk += block_size)
    for (jj = 0; jj < length; jj += block_size)
      for (i = 0; i < length; i++)
	for (j = jj; j< jj+block_size; j++) {
	  sum = c0[i*length+j];
	  for (k = kk; k < kk+block_size; k++)
	    sum += a0[i*length+k] * b0[k*length+j];
	  c0[i*length+j] += sum;
	}
}

