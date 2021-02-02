/*****************************************************************************/
// gcc -O1 -o test_transpose test_transpose.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.4         // Cycles per GHz -- Adjust to your computer

#define BASE  0
#define ITERS 20
#define DELTA 300


#define BBASE  10
#define BDELTA 100

#define OPTIONS 5

/*****************************************************************************/
main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  int init_vector(float* v, long int len);
  void transpose(float* v0, float* v1, long int length);
  void transpose_rev(float* v0, float* v1, long int length);
  void transpose_block(float* v0, float* v1, long int length, long int bsize);

  long int i, j, len, bsize;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  printf("\n Hello World -- Transpose \n");

  // declare and initialize the vector structures
  float* v0 = (float*) malloc (MAXSIZE*MAXSIZE*sizeof(float));
  float* v1 = (float*) malloc (MAXSIZE*MAXSIZE*sizeof(float));
  init_vector(v0, MAXSIZE);  init_vector(v1, MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    len = BASE+(i+1)*DELTA;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    transpose(v0, v1, len);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION ++;
  for (i = 0; i < ITERS; i++) {
    len = BASE+(i+1)*DELTA;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    transpose_rev(v0, v1, len);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
 
  OPTION ++;
  bsize = BBASE;
  for (i = 0; i < ITERS; i++) {
    len = BASE+(i+1)*DELTA;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    transpose_block(v0, v1, len, bsize);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
  
  OPTION ++;
  bsize += BDELTA;
  for (i = 0; i < ITERS; i++) {
    len = BASE+(i+1)*DELTA;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    transpose_block(v0, v1, len, bsize);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
  
  OPTION ++;
  bsize += BDELTA;
  for (i = 0; i < ITERS; i++) {
    len = BASE+(i+1)*DELTA;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    transpose_block(v0, v1, len, bsize);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  /* output times */
  printf("\nsize,ij, ji, block size = %d, block size = %d, block size = %d", BBASE, BBASE+BDELTA, BBASE+2*BDELTA);  
  for (i = 0; i < ITERS; i++) {
    printf("\n%d, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }

printf("\n");
  
}/* end main */
/*********************************/

/* initialize 2D vector */
int init_vector(float* v, long int len)
{
  long int i;

  if (len > 0) {
    for (i = 0; i < len*len; i++) v[i] = (float)(i);
    return 1;
  }
  else return 0;
}

/************************************/

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

/************************************/

/* transpose */
void transpose(float* v0, float* v1, long int length)
{
  long int i, j;
  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      v1[j*length+i] = v0[i*length+j];
}


void transpose_rev(float* v0, float* v1, long int length){
  
  long int i, j;
  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      v1[i*length+j] = v0[j*length+i];

  
  }

void transpose_block(float* v0, float* v1, long int length, long int bsize){

  long int i, j, k, l;
  int n = length;
  for ( i = 0; i < n; i += bsize) 
    for ( j = 0; j < n; j += bsize) 
        for ( k = i; k < i + bsize; ++k) 
            for ( l = j; l < j + bsize; ++l) 
                v1[k + l*n] = v0[l + k*n];

            
        

}
