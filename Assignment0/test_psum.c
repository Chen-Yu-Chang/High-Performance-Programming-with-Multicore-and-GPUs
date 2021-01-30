/******************************************************************************/

// gcc -O0 -o test_psum test_psum.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define SIZE 10000000
#define ITERS 20
#define DELTA 10

/******************************************************************************/
main(int argc, char *argv[])
{
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp[2*ITERS+1], times[2*ITERS+1];
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    void psum1(float a[], float p[], long int n);
    void psum2(float a[], float p[], long int n);
    float *in, *out;
    long int i, j, k;
    
    // initialize
    in = (float *) malloc(SIZE * sizeof(*in));
    out = (float *) malloc(SIZE * sizeof(*out));
    for (i = 0; i < SIZE; i++) in[i] = (float)(i);
    
    // process psum1 for various array sizes and collect timing
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stamp[0]);
    for (i = 0; i < ITERS; i++) {
        psum1(in, out, DELTA*(i+1));
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stamp[i+1]);
    }
    
    // process psum2 for various array sizes
    for (; i < 2*ITERS; i++) {
        psum2(in, out, DELTA*(i-ITERS+1));
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stamp[i+1]);
    }
    
    // output timing
    for (i = 0; i < ITERS; i++) {
        printf("\n %d", (i+1)*DELTA);
        printf(",  %d", diff(time_stamp[i],time_stamp[i+1]).tv_nsec);
        printf(",  %d", diff(time_stamp[i+ITERS],time_stamp[i+ITERS+1]).tv_nsec);
    }
    
    printf("\n");
}/* end main */


void psum1(float a[], float p[], long int n)
{
    long int i;
    
    p[0] = a[0];
    for (i = 1; i < n; i++)
        p[i] = p[i-1] + a[i];
    
}

void psum2(float a[], float p[], long int n)
{
    long int i;
    
    p[0] = a[0];
    for (i = 1; i < n-1; i+=2) {
        float mid_val = p[i-1] + a[i];
        p[i] = mid_val;
        p[i+1] = mid_val + a[i+1];
    }
    
    /* For odd n, finish remaining element */
    if (i < n)
        p[i] = p[i-1] + a[i];
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

