/*****************************************************************************/
// gcc -O1 -o part4_final part4_final.c -lm -lrt

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define OPTIONS 2
#define BASE  0
#define ITERS 1
#define DELTA 200        // Right now only set up to do one size

#define BLOCK_SIZE 4

#define MINVAL   0.0
#define MAXVAL  100.0

#define TOL 0.00001

#define O_ITERS 150        // # of OMEGA values to be tested
#define PER_O_ITERS 10    // trials per OMEGA value
double OMEGA = 1.90;     // OMEGA base - first OMEGA tested
#define OMEGA_INC 0.01   // OMEGA increment for each O_ITERS
int no_rows_per_thread;
#define NUM_THREADS 4
long int iterations = 0;
long int iterations_block = 0;
typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
    long int len;
    data_t *data;
} vec_rec, *vec_ptr;

struct thread_data{
    vec_ptr vec;
    long int arg;
};

/*****************************************************************************/
main(int argc, char *argv[])
{
    int OPTION=0;
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp[OPTIONS][ITERS+1];
    int convergence[OPTIONS][ITERS+1];
    //double convergence[O_ITERS][2];
    vec_ptr new_vec(long int len);
    int set_vec_length(vec_ptr v, long int index);
    long int get_vec_length(vec_ptr v);
    int init_vector(vec_ptr v, long int len);
    int init_vector_rand(vec_ptr v, long int len);
    int print_vector(vec_ptr v);
    void *SOR(void *arg);
    void* status;
    
    void *SOR_blocked(void* arg);
    long int i, j, k;
    long int time_sec, time_ns;
    long int MAXSIZE = (BASE+(ITERS)*DELTA);
    no_rows_per_thread = MAXSIZE/NUM_THREADS;
    
    
    pthread_t Threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];
    
    
    vec_ptr v0 = new_vec(MAXSIZE);
    
    
    
    
    OPTION = 0;
    for (j = 0; j < ITERS; j++) {
        // printf("\niter = %d length = %d OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
        set_vec_length(v0, MAXSIZE);
        init_vector_rand(v0, MAXSIZE);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        for(i=0;i<NUM_THREADS; i++)
        {
            data[i].vec = v0;
            data[i].arg = i;
            pthread_create(&Threads[i], NULL, SOR, (void *)&data[i]);
        }
        for(i=0; i<NUM_THREADS; i++)
        {
            pthread_join(Threads[i], &status);
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][j] = diff(time1,time2);
        convergence[OPTION][j] = iterations;
        //print_vector(v0);
        printf("Loop no: %ld\t", j);
    }
    
    
    OPTION ++;
    for (j = 0; j < ITERS; j++) {
        // printf("\niter = %d length = %d OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
        set_vec_length(v0, MAXSIZE);
        init_vector_rand(v0, MAXSIZE);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        for(i=0;i<NUM_THREADS; i++)
        {
            data[i].vec = v0;
            data[i].arg = i;
            pthread_create(&Threads[i], NULL, SOR_blocked, (void *)&data[i]);
        }
        for(i=0; i<NUM_THREADS; i++)
        {
            pthread_join(Threads[i], &status);
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][j] = diff(time1,time2);
        convergence[OPTION][j] = iterations_block;
        //print_vector(v0);
    }
    
    
    //  printf("\n%ld\n", iterations);
    // printf("\n%ld\n", iterations_block);
    for (i = 0; i < ITERS; i++) {
        printf("\n%d, ", (BASE+(i+1)*DELTA));
        for (j = 0; j < OPTIONS; j++) {
            if (j != 0) printf(", ");
            printf("%ld", (long int)((double)(CPG)*(double)
                                     (GIG*time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec))/NUM_THREADS);
            printf(", %d", convergence[j][i]);
        }
    }
    printf("\n");
    
}/* end main */
/*********************************/

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
/* Create 2D vector of specified length per dimension */
vec_ptr new_vec(long int len)
{
    long int i;
    
    /* Allocate and declare header structure */
    vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
    if (!result) return NULL;  /* Couldn't allocate storage */
    result->len = len;
    
    /* Allocate and declare array */
    if (len > 0) {
        data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
        if (!data) {
            free((void *) result);
            printf("\n COULDN'T ALLOCATE STORAGE \n", result->len);
            return NULL;  /* Couldn't allocate storage */
        }
        result->data = data;
    }
    else result->data = NULL;
    
    return result;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
    v->len = index;
    return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
    return v->len;
}

/* initialize 2D vector */
int init_vector(vec_ptr v, long int len)
{
    long int i;
    
    if (len > 0) {
        v->len = len;
        for (i = 0; i < len*len; i++) v->data[i] = (data_t)(i);
        return 1;
    }
    else return 0;
}

/* initialize vector with random number in a range */
int init_vector_rand(vec_ptr v, long int len)
{
    long int i;
    double fRand(double fMin, double fMax);
    
    if (len > 0) {
        v->len = len;
        for (i = 0; i < len*len; i++)
            v->data[i] = (data_t)(fRand((double)(MINVAL),(double)(MAXVAL)));
        return 1;
    }
    else return 0;
}

/* print vector for test */
int print_vector(vec_ptr v)
{
    long int i, j, len;
    
    len = v->len;
    printf("\n length = %ld", len);
    for (i = 0; i < len; i++) {
        printf("\n");
        for (j = 0; j < len; j++)
            printf("%.4f ", (data_t)(v->data[i*len+j]));
    }
}

data_t *get_vec_start(vec_ptr v)
{
    return v->data;
}

/************************************/

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/************************************/

/* SOR */
void *SOR(void *arg)
{
    
    //  printf("In the thread\n");
    long int i, j;
    struct thread_data *data_thread = (struct thread_data*)arg;
    vec_ptr v0 = data_thread->vec;
    long int length = get_vec_length(v0);
    data_t *data = get_vec_start(v0);
    double change, mean_change = 100;   // start w/ something big
    int iters = 0;
    
    long int offset = data_thread->arg;
    
    long int start = offset*no_rows_per_thread;
    long int end = start + no_rows_per_thread;
    
    while ((mean_change/(double)(length*length)) > (double)TOL) {
        iters++;
        mean_change = 0;
        for (i = start+1; i < end-1; i++)
            for (j = 1; j < length-1; j++) {
                change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
                                                   data[(i+1)*length+j] +
                                                   data[i*length+j+1] +
                                                   data[i*length+j-1]);
                data[i*length+j] -= change * OMEGA;
                if (change < 0){
                    change = -change;
                }
                mean_change += change;
            }
        if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
            //     printf("\n PROBABLY DIVERGENCE iter = %ld", iters);
            break;
        }
    }
    iterations += iters;
}


/* SOR w/ blocking */
void *SOR_blocked(void* arg)
{
    long int i, j, ii, jj;
    struct thread_data *data_thread = (struct thread_data*)arg;
    vec_ptr v0 = data_thread->vec;
    long int length = get_vec_length(v0);
    data_t *data = get_vec_start(v0);
    double change, mean_change = 100;
    int iters = 0;
    int k;
    
    long int offset = data_thread->arg;
    
    long int start = offset*no_rows_per_thread;
    long int end = start + no_rows_per_thread;
    
    while ((mean_change/(double)(length*length)) > (double)TOL) {
        iters++;
        mean_change = 0;
        for (ii = start+1; ii < end-1; ii+=BLOCK_SIZE)
            for (jj = 1; jj < length-1; jj+=BLOCK_SIZE)
                for (i = ii; i < ii+BLOCK_SIZE; i++)
                    for (j = jj; j < jj+BLOCK_SIZE; j++) {
                        change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
                                                           data[(i+1)*length+j] +
                                                           data[i*length+j+1] +
                                                           data[i*length+j-1]);
                        data[i*length+j] -= change * OMEGA;
                        if (change < 0){
                            change = -change;
                        }
                        mean_change += change;
                    }
        if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
            printf("\n PROBABLY DIVERGENCE iter = %d", iters);
            break;
        }
    }
    iterations_block += iters;
}
