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
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec t1, t2;
    long long int *p1, *p2;
    long long int sec, ns;
    
    // initialize
    in = (float *) malloc(MAX_SIZE * sizeof(*in));
    out = (float *) malloc(MAX_SIZE * sizeof(*out));
    for (i = 0; i < MAX_SIZE; i++) in[i] = (float)(i);
    p1 = malloc(MAX_SIZE * sizeof(*p1));
    p2 = malloc(MAX_SIZE * sizeof(*p2));
    
    // process psum1 for various array sizes and collect timing
    for (j = 0; j < 10000000; j+=100) {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
        psum1(in, out, j);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
        printf("psum: %f\n", out[j]);
        sec = diff(t1, t2).tv_sec;
        ns = diff(t1, t2).tv_nsec;
        p1[j] = ns;
    }
    
    // process psum2 for various array sizes and collect timing
    for (j = 0; j < 10000000; j+=100) {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
        psum2(in, out, j);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
        printf("psum: %f\n", out[j]);
        sec = diff(t1, t2).tv_sec;
        ns = diff(t1, t2).tv_nsec;
        p2[j] = ns;
    }
    
    // output timing
    for (k = 0; k < 10000000; k+=100) {
        printf("%ld, %ld, %ld, %ld\n", k, p1[k], k, p2[k]);
    }
    
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
