/*****************************************************************************/
// gcc -O1 -pthread -o test_SOR_multithreaded test_SOR_multithreaded.c -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
#define CPG 2.53          // Cycles per GHz -- Adjust to your computer

#define BASE  2
#define ITERS 1
#define DELTA 1250

#define OPTIONS 2
#define BLOCK_SIZE 8     // TO BE DETERMINED

#define MINVAL   0.0
#define MAXVAL  10.0

#define TOL 0.00001
#define OMEGA 1.60       // TO BE DETERMINED

typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
    long int len;
    data_t *data;
} vec_rec, *vec_ptr;

/* Number of bytes in a vector (SSE sense) */
#define VBYTES 16

/* Number of elements in a vector (SSE sense) */
#define VSIZE VBYTES/sizeof(data_t)

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
    vec_t v;
    data_t d[VSIZE];
} pack_t;

/* used to pass parameters to worker threads */
struct thread_data{
    int thread_id;
    vec_ptr v;
    int *interactions;
};

#define NUM_THREADS 2

/*****************************************************************************/
main(int argc, char *argv[])
{
    int OPTION;
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp[OPTIONS][ITERS+1];
    int convergence[OPTIONS][ITERS+1];
    vec_ptr new_vec(long int len);
    int set_vec_length(vec_ptr v, long int index);
    long int get_vec_length(vec_ptr v);
    int init_vector(vec_ptr v, long int len);
    int init_vector_rand(vec_ptr v, long int len);
    int print_vector(vec_ptr v);
    int *iterations;
    
    void pt_SOR(vec_ptr v, int *iterations)
    void SOR(vec_ptr v, int *iterations);
    void SOR_ji(vec_ptr v, int *iterations);
    void SOR_blocked(vec_ptr v, int *iterations);
    
    long int i, j, k;
    long int time_sec, time_ns;
    long int MAXSIZE = BASE+(ITERS+1)*DELTA;
    
    printf("\n Hello World -- SOR serial variations \n");
    
    // declare and initialize the vector structure
    vec_ptr v0 = new_vec(MAXSIZE);
    iterations = (int *) malloc(sizeof(int));
    
    OPTION = 0;
    for (i = 0; i < ITERS; i++) {
        printf("\niter = %d length = %d OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
        set_vec_length(v0, BASE+(i+1)*DELTA);
        init_vector_rand(v0, BASE+(i+1)*DELTA);
        //print_vector(v0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        SOR(v0,iterations);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][i] = diff(time1,time2);
        convergence[OPTION][i] = *iterations;
        //print_vector(v0);
    }
    
    OPTION++;
    for (i = 0; i < ITERS; i++) {
        printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
        init_vector_rand(v0, BASE+(i+1)*DELTA);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        set_vec_length(v0,BASE+(i+1)*DELTA);
        pt_SOR(v0,iterations);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][i] = diff(time1,time2);
        convergence[OPTION][i] = *iterations;
    }
    
    
    for (i = 0; i < ITERS; i++) {
        printf("\n%d, %d, ", BASE+(i+1)*DELTA, BLOCK_SIZE);
        for (j = 0; j < OPTIONS; j++) {
            if (j != 0) printf(", ");
            printf("%ld", (long int)((double)(CPG)*(double)
                                     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
            printf(", %d", convergence[j][i]);
        }
    }
    
    printf("\n");
    
}/* end main */
/*********************************/

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

/* initialize vector with another */
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

/* print vector */
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

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/************************************/

/* SOR */
void SOR(vec_ptr v, int *iterations)
{
    long int i, j;
    long int length = get_vec_length(v);
    data_t *data = get_vec_start(v);
    double change, mean_change = 100;   // start w/ something big
    int iters = 0;
    
    while ((mean_change/(double)(length*length)) > (double)TOL) {
        iters++;
        mean_change = 0;
        for (i = 1; i < length-1; i++)
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
            printf("\n PROBABLY DIVERGENCE iter = %ld", iters);
            break;
        }
    }
    *iterations = iters;
    printf("\n iters = %d", iters);
}

void *SOR_work(void *threadarg){
    long int i, j, low, high;
    struct thread_data = *my_data;
    my_data = (struct thread_data * threadarg;
               int taskid = my_data->thread_id;
               vec_ptr v = my_data->v;
               long int length = get_vec_length(v);
               data_t *data = get_vec_start(v);
               double change, mean_change = 100;   // start w/ something big
               int iters = 0;
               
               low = (taskid*length)/NUM_THREADS;
               high = ((taskid+1)*length)/NUM_THREADS;
               
               while ((mean_change/(double)(length*length)) > (double)TOL) {
                   iters++;
                   mean_change = 0;
                   for (i = low; i < high; i++)
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
                       printf("\n PROBABLY DIVERGENCE iter = %ld", iters);
                       break;
                   }
               }
               *iterations = iters;
               }
               
               void pt_SOR(vec_ptr v, int *iterations)
    {
        pthread_t threads[NUM_THREADS];
        struct thread_data thread_data_array[NUM_THREADS];
        long t;
        int rc;
        int iters = 0;
        
        for(t = 0; t < NUM_THREADS; t++){
            thread_data_array[t].thread_id = t;
            thread_data_array[t].v = v;
            thread_data_array[t].interactions = interactions;
            rc = pthread_create(&threads[t], NULL, SOR_work, (void*) & thread_data_array[t]);
            if(rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
        
        
        for(t = 0; t < NUM_THREADS; t++){
            if(pthread_join(threads[t], NULL)){
                printf("ERROR; code on return from join is %d\n", rc);
                exit(-1);
            } else{
                iters += thread_data_array[t].iterations;
            }
        }
        
        printf("\n iters = %d", iters);
    }
