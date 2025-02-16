/*****************************************************************************/
// gcc -O1 -mavx test_transpose.c -lrt -o test_transpose

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#define CPNS 2.4    /* Cycles per nanosecond -- Adjust to your computer,
for example a 3.2 GHz GPU, this would be 3.2 */

/* We want to test a range of work sizes. We will generate these
 using the q8   adratic formula:  A x^2 + B x + C                     */
#define A   8  /* coefficient of x^2 */
#define B   8  /* coefficient of x */
#define C   8  /* constant term */

#define NUM_TESTS 10

#define OPTIONS 3       // ij and ji -- need to add other methods

typedef float data_t;

/* Create abstract data type for vector */
typedef struct {
    long int rowlen;
    data_t *data;
} array_rec, *array_ptr;

/* Number of bytes in a vector */
#define VBYTES 32

/* Number of elements in a vector */
#define VSIZE (VBYTES/sizeof(data_t))



array_ptr new_array(long int rowlen);
int set_array_rowlen(array_ptr v, long int index);
long int get_array_rowlen(array_ptr v);
int init_array(array_ptr v, long int len);
void transpose(array_ptr v0, array_ptr v1);
void transpose_rev(array_ptr v0, array_ptr v1);
void transpose_sse_blocking(array_ptr v0, array_ptr v1);


/* -=-=-=-=- Time measurement by clock_gettime() -=-=-=-=- */
/*
 As described in the clock_gettime manpage (type "man clock_gettime" at the
 shell prompt), a "timespec" is a structure that looks like this:
 
 struct timespec {
 time_t   tv_sec;   // seconds
 long     tv_nsec;  // and nanoseconds
 };
 */

double interval(struct timespec start, struct timespec end)
{
    struct timespec temp;
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    if (temp.tv_nsec < 0) {
        temp.tv_sec = temp.tv_sec - 1;
        temp.tv_nsec = temp.tv_nsec + 1000000000;
    }
    return (((double)temp.tv_sec) + ((double)temp.tv_nsec)*1.0e-9);
}
/*
 This method does not require adjusting a #define constant
 
 How to use this method:
 
 struct timespec time_start, time_stop;
 clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
 // DO SOMETHING THAT TAKES TIME
 clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
 measurement = interval(time_start, time_stop);
 
 */


/* -=-=-=-=- End of time measurement declarations =-=-=-=- */


/* This routine "wastes" a little time to make sure the machine gets
 out of power-saving mode (800 MHz) and switches to normal speed. */
double wakeup_delay()
{
    double meas = 0; int i, j;
    struct timespec time_start, time_stop;
    double quasi_random = 0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
    j = 100;
    while (meas < 1.0) {
        for (i=1; i<j; i++) {
            /* This iterative calculation uses a chaotic map function, specifically
             the complex quadratic map (as in Julia and Mandelbrot sets), which is
             unpredictable enough to prevent compiler optimisation. */
            quasi_random = quasi_random*quasi_random - 1.923432;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        meas = interval(time_start, time_stop);
        j *= 2;
    }
    return quasi_random;
}


/*****************************************************************************/
int main(int argc, char *argv[])
{
    int OPTION;
    struct timespec time_start, time_stop;
    double time_stamp[OPTIONS][NUM_TESTS];
    data_t *data_holder;
    long int k;
    double wd;
    long int x, n, alloc_size;
    
    printf("Transpose (lab3)\n");
    
    wd = wakeup_delay();
    x = NUM_TESTS-1;
    alloc_size = A*x*x + B*x + C;
    
    /* declare and initialize the vector structures */
    array_ptr v0 = new_array(alloc_size);  init_array(v0, alloc_size);
    array_ptr v1 = new_array(alloc_size);  init_array(v1, alloc_size);
    
    OPTION = 0;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_array_rowlen(v0, n);
        set_array_rowlen(v1, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        transpose(v0, v1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_array_rowlen(v0, n);
        set_array_rowlen(v1, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        transpose_rev(v0, v1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_array_rowlen(v0, n);
        set_array_rowlen(v1, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        transpose_sse_blocking(v0, v1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    /* output times */
    printf("size,   ij,   ji,    blocking \n");
    {
        int i, j;
        for (i = 0; i < x; i++) {
            printf("%ld,  ", A*i*i + B*i + C);
            for (j = 0; j < OPTIONS; j++) {
                if (j != 0) {
                    printf(", ");
                }
                printf("%ld", (long int)((double)(CPNS) * 1.0e9 * time_stamp[j][i]));
            }
            printf("\n");
        }
    }
    
    printf("\n");
    printf("Wakeup delay calculated %f\n", wd);
    
} /* end main */
/*********************************/

/* Create 2D vector of specified length per dimension */
array_ptr new_array(long int rowlen)
{
    long int i;
    
    /* Allocate and declare header structure */
    array_ptr result = (array_ptr) malloc(sizeof(array_rec));
    if (!result) return NULL;  /* Couldn't allocate storage */
    result->rowlen = rowlen;
    
    if (rowlen % VSIZE) {
        fprintf(stdout,
                "new_array: row length %ld is not a multiple of vector size (VSIZE=%d)\n"
                "to fix this, A, B and C must all be a multiple of %d\n",
                rowlen, ((int)VSIZE), ((int)VSIZE));
        exit(-1);
    }
    
    /* Allocate and declare array */
    if (rowlen > 0) {
        data_t *data = (data_t *) calloc(rowlen*rowlen+VSIZE, sizeof(data_t));
        if (!data) {
            /* Couldn't allocate storage */
            free((void *) result);
            fprintf(stderr, " COULDN'T ALLOCATE %ld BYTES STORAGE \n",
                    (rowlen*rowlen+VSIZE)*sizeof(data_t));
            exit(-1);
        }
        while (((long)data) % (VSIZE * sizeof(data_t)) != 0) {
            data++;
        }
        result->data = data;
    }
    else result->data = NULL;
    
    return result;
}

/* Set length of vector */
int set_array_rowlen(array_ptr v, long int index)
{
    v->rowlen = index;
    return 1;
}

/* Return length of vector */
long int get_array_rowlen(array_ptr v)
{
    return v->rowlen;
}

/* initialize 2D vector */
int init_array(array_ptr v, long int len)
{
    long int i;
    
    if (len > 0) {
        v->rowlen = len;
        for (i = 0; i < len*len; i++) {
            v->data[i] = (data_t)(i);
        }
        return 1;
    }
    else return 0;
}

data_t *get_array_start(array_ptr v)
{
    return v->data;
}


/************************************/

/* transpose */
void transpose(array_ptr v0, array_ptr v1)
{
    long int i, j;
    long int get_array_rowlen(array_ptr v);
    data_t *get_array_start(array_ptr v);
    long int length = get_array_rowlen(v0);
    data_t *data0 = get_array_start(v0);
    data_t *data1 = get_array_start(v1);
    
    for (i = 0; i < length; i++) {
        for (j = 0; j < length; j++) {
            data1[j*length+i] = data0[i*length+j];
        }
    }
}

/* transpose with loops interchanged */
void transpose_rev(array_ptr v0, array_ptr v1)
{
    long int i, j;
    long int get_array_rowlen(array_ptr v);
    data_t *get_array_start(array_ptr v);
    long int length = get_array_rowlen(v0);
    data_t *data0 = get_array_start(v0);
    data_t *data1 = get_array_start(v1);
    
    for (j = 0; j < length; j++) {
        for (i = 0; i < length; i++) {
            data1[j*length+i] = data0[i*length+j];
        }
    }
}

/* transpose sse blocking */
void transpose_sse_blocking(array_ptr v0, array_ptr v1)
{
    long int i, j, m, n;
    __m128   m1, m2, m3, m4, m5, m6, m7;
    __m128* pSrc1;
    __m128* pSrc2;
    
    data_t *data0 = get_array_start(v0);
    data_t *data1 = get_array_start(v1);
    
    pSrc1 = (__m128*)(data0);
    pSrc2 = (__m128*)(data1);
    
    
    long int get_array_rowlen(array_ptr v);
    long int length = get_array_rowlen(v0)/4;
    
    for (i = 0; i < length; ++i){
        for(j =0; j< length ; ++j){
            
            
            m1 = (*pSrc1);
            pSrc1++;
            m2 = (*pSrc1);
            pSrc1++;
            m3 = (*pSrc1);
            pSrc1++;
            m4 = (*pSrc1);
            _MM_TRANSPOSE4_PS(m1,m2,m3,m4);
            pSrc1++;
            (*pSrc2) = m1;
            pSrc2++;
            (*pSrc2) = m2;
            pSrc2++;
            (*pSrc2) = m3;
            pSrc2++;
            (*pSrc2) = m4;
            pSrc2++;
        }
    }
}
