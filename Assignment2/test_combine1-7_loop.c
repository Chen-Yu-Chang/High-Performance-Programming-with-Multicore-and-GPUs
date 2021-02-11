/*****************************************************************************/

// gcc -O1 test_combine1-7.c -lrt -o test_combine

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

/* We want to test a range of work sizes. We will generate these
 using the quadratic formula:  A x^2 + B x + C                     */
#define A   3   /* coefficient of x^2 */
#define B   2   /* coefficient of x */
#define C   10  /* constant term */

#define NUM_TESTS 10   /* Number of different sizes to test */


#define OUTER_LOOPS 2000

#define GIG 1000000000
#define CPNS 3.0    /* Cycles per nanosecond -- Adjust to your computer,
for example a 3.2 GhZ GPU, this would be 3.2 */

#define OPTIONS 7

/* Type of operation. This can be multiplication or addition.
 for addition, IDENT should be 0.0 and OP should be +
 for multiplication, IDENT should be 1.0 and OP should be *
 */
#define IDENT 0.0
#define OP +

/* Type of data being "combined". This can be any of the types:
 int, long int, float, double, long double */
typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
    long int len;
    data_t *data;
} vec_rec, *vec_ptr;

/* Prototypes */
vec_ptr new_vec(long int len);
int get_vec_element(vec_ptr v, long int index, data_t *dest);
long int get_vec_length(vec_ptr v);
int set_vec_length(vec_ptr v, long int index);
int init_vector(vec_ptr v, long int len);
void combine1(vec_ptr v, data_t *dest);
void combine2(vec_ptr v, data_t *dest);
void combine3(vec_ptr v, data_t *dest);
void combine4(vec_ptr v, data_t *dest);
void combine5(vec_ptr v, data_t *dest);
void combine6(vec_ptr v, data_t *dest);
void combine7(vec_ptr v, data_t *dest);


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
    double final_answer = 0;
    long int x, n, k, alloc_size;
    data_t *result;
    
    printf("Vector reduction (combine) examples\n");
    
    final_answer = wakeup_delay();
    
    x = NUM_TESTS-1;
    alloc_size = A*x*x + B*x + C;
    
    /* declare and initialize the vector structure */
    vec_ptr v0 = new_vec(alloc_size);
    init_vector(v0, alloc_size);
    result = (data_t *) malloc(sizeof(data_t));
    
    printf("Testing %d variants of combine(), on vectors of %d sizes from %d to %d\n",
           OPTIONS, NUM_TESTS, C, alloc_size);
    
    /* execute and time all 7 options from B&O  */
    OPTION = 0;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine1(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine2(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine3(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine4(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine5(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine6(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    OPTION++;
    printf("testing option %d\n", OPTION);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        set_vec_length(v0, n);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
        for(k=0; k<OUTER_LOOPS; k++) {
            combine7(v0, result);
            final_answer += *result;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_stop);
        time_stamp[OPTION][x] = interval(time_start, time_stop);
    }
    
    /* output times */
    printf("%s", "size, combine1, combine2, combine3, combine4, combine5, combine6, combine7\n");
    {
        int i, j;
        for (i = 0; i < NUM_TESTS; i++) {
            printf("%d,  ", (A*i*i + B*i + C) * OUTER_LOOPS );
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
    printf("Sum of all results: %g\n", final_answer);
    
    return 0;
} /* end main */

/**********************************************/
/* Create vector of specified length */
vec_ptr new_vec(long int len)
{
    long int i;
    
    /* Allocate and declare header structure */
    vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
    if (!result) return NULL;  /* Couldn't allocate storage */
    result->len = len;
    
    /* Allocate and declare array */
    if (len > 0) {
        data_t *data = (data_t *) calloc(len, sizeof(data_t));
        if (!data) {
            free((void *) result);
            return NULL;  /* Couldn't allocate storage */
        }
        result->data = data;
    }
    else result->data = NULL;
    
    return result;
}

/* Retrieve vector element and store at dest.
 Return 0 (out of bounds) or 1 (successful)
 */
int get_vec_element(vec_ptr v, long int index, data_t *dest)
{
    if (index < 0 || index >= v->len) {
        return 0;
    }
    *dest = v->data[index];
    return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
    return v->len;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
    v->len = index;
    return 1;
}

/* initialize vector */
int init_vector(vec_ptr v, long int len)
{
    long int i;
    
    if (len > 0) {
        v->len = len;
        for (i = 0; i < len; i++) {
            v->data[i] = (data_t)(i+1);
        }
        return 1;
    }
    else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
    return v->data;
}


/*************************************************/
/* Combine1:  Accumulate result in local variable
 * Example of --> Eliminate unneeded memory references */
void combine1(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    for (i = 0; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}

/* Combine2:  Unroll loop by 2
 * Example of --> Loop unrolling */
void combine2(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    /* Combine two elements at a time */
    for (i = 0; i < limit; i+=2) {
        acc = (acc OP data[i]) OP data[i+1];
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}

/* Combine3:  Unroll loop by 6
 * Example of --> Loop unrolling */
void combine3(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    /* Combine two elements at a time */
    for (i = 0; i < limit; i+=6) {
        acc = (acc OP data[i]) OP data[i+1];
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}

/* Combine4:  Unroll loop by 8
 * Example of --> Loop unrolling */
void combine4(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    /* Combine two elements at a time */
    for (i = 0; i < limit; i+=8) {
        acc = (acc OP data[i]) OP data[i+1];
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}
/* Combine5:  Unroll loop by 10
 * Example of --> Loop unrolling */
void combine5(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    /* Combine two elements at a time */
    for (i = 0; i < limit; i+=10) {
        acc = (acc OP data[i]) OP data[i+1];
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}

/* Combine6:  Unroll loop by 2, 2 accumulators
 * Example of --> parallelism */
void combine6(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc0 = IDENT;
    data_t acc1 = IDENT;
    
    /* Combine two elements at a time w/ 2 accumulators */
    for (i = 0; i < limit; i+=2) {
        acc0 = acc0 OP data[i];
        acc1 = acc1 OP data[i+1];
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc0 = acc0 OP data[i];
    }
    *dest = acc0 OP acc1;
}

/* Combine7:  Unroll loop by 2, change associativity
 * Example of --> parallelism */
void combine7(vec_ptr v, data_t *dest)
{
    long int i;
    long int get_vec_length(vec_ptr v);
    data_t *get_vec_start(vec_ptr v);
    long int length = get_vec_length(v);
    long int limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    
    /* Combine two elements at a time */
    for (i = 0; i < limit; i+=2) {
        acc = acc OP (data[i] OP data[i+1]);
    }
    
    /* Finish remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }
    *dest = acc;
}
