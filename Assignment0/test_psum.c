/****************************************************************************/
// gcc -O0 test_psum.c -lm -lrt -o test_psum

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_SIZE 10000000

/****************************************************************************/
int main(int argc, char *argv[])
{
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    void psum1(float a[], float p[], long int n);
    void psum2(float a[], float p[], long int n);
    float *in, *out;
    long int i, j, k;
    
    // initialize
    in = (float *) malloc(MAX_SIZE * sizeof(*in));
    out = (float *) malloc(MAX_SIZE * sizeof(*out));
    for (i = 0; i < MAX_SIZE; i++) in[i] = (float)(i);
    
    // process psum1 for various array sizes and collect timing
    
    
    // process psum2 for various array sizes and collect timing
    
    
    // output timing
    
    
} /* end of main() */


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
