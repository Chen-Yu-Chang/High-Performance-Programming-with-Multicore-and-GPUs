/*************************************************************************/
// gcc -pthread test_pt.c -lm -lrt -o test_pt

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "apple_pthread_barrier.h"

#define CPG 2.4           // Cycles per GHz -- Adjust to your computer

#define A   8   /* coefficient of x^2 */
#define B   16  /* coefficient of x */
#define C   32  /* constant term */
#define NUM_TESTS 5
#define OPTIONS 2        // Current setting, vary as you wish!
#define IDENT 0

#define INIT_LOW -10.0
#define INIT_HIGH 10.0

#define TOL 0.00001

typedef double data_t;

/* Create abstract data type for matrix */
typedef struct {
    long int len;
    data_t *data;
} matrix_rec, *matrix_ptr;

int NUM_THREADS = 4;

/* used to pass parameters to worker threads */
struct thread_data{
    int thread_id;
    matrix_ptr a;
    
};
double differ;
pthread_mutex_t diff_lock;
pthread_barrier_t bar1;

/*************************************************************************/
int main(int argc, char *argv[])
{
    int OPTION;
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp[OPTIONS][ITERS+1];
    matrix_ptr new_matrix(long int len);
    int set_matrix_length(matrix_ptr m, long int index);
    long int get_matrix_length(matrix_ptr m);
    int init_matrix(matrix_ptr m, long int len);
    int init_matrix_rand(matrix_ptr m, long int len);
    int init_matrix_rand_grad(matrix_ptr m, long int len);
    int zero_matrix(matrix_ptr m, long int len);
    void pt_cb_base(matrix_ptr a, matrix_ptr b, matrix_ptr c);
    void pt_cb_pthr(matrix_ptr a, matrix_ptr b, matrix_ptr c);
    void pt_mb(matrix_ptr a, matrix_ptr b, matrix_ptr c, matrix_ptr d);
    void pt_ob(matrix_ptr a, matrix_ptr b, matrix_ptr c);
    
    long int i, j, k;
    long int time_sec, time_ns;
    
    printf(" Hello World -- Test SOR pthreads \n");
    
    // declare and initialize the matrix structure
    matrix_ptr a0 = new_matrix(MAXSIZE);
    init_matrix_rand(a0, MAXSIZE);
    matrix_ptr b0 = new_matrix(MAXSIZE);
    init_matrix_rand(b0, MAXSIZE);
    matrix_ptr c0 = new_matrix(MAXSIZE);
    zero_matrix(c0, MAXSIZE);
    matrix_ptr d0 = new_matrix(MAXSIZE);
    init_matrix_rand_grad(d0, MAXSIZE);
    
    /*OPTION = 0;
     printf("OPTION %d - pt_cb_base()\n", OPTION);
     for (i = 0; i < ITERS; i++) {
     init_matrix_rand(a0,BASE+(i+1)*DELTA);
     set_matrix_length(a0,BASE+(i+1)*DELTA);
     set_matrix_length(b0,BASE+(i+1)*DELTA);
     set_matrix_length(c0,BASE+(i+1)*DELTA);
     clock_gettime(CLOCK_REALTIME, &time1);
     pt_cb_base(a0,b0,c0);
     clock_gettime(CLOCK_REALTIME, &time2);
     time_stamp[OPTION][i] = diff(time1,time2);
     printf("iter %d done\n", i);
     }*/
    
    NUM_THREADS = 16;
    OPTION =0;
    printf("OPTION %d: pt_cb_pthr() with %d threads\n", OPTION, NUM_THREADS);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        init_matrix_rand(a0,n);
        set_matrix_length(a0,n);
        set_matrix_length(b0,n);
        set_matrix_length(c0,n);
        clock_gettime(CLOCK_REALTIME, &time1);
        pt_cb_pthr(a0,b0,c0);
        clock_gettime(CLOCK_REALTIME, &time2);
        time_stamp[OPTION][x] = diff(time1,time2);
        printf("iter %d done\n", x);
    }
    
    NUM_THREADS = 4;
    OPTION++;
    printf("OPTION %d: pt_cb_pthr() with %d threads\n", OPTION, NUM_THREADS);
    for (x=0; x<NUM_TESTS && (n = A*x*x + B*x + C, n<=alloc_size); x++) {
        init_matrix_rand(a0,n);
        set_matrix_length(a0,n);
        set_matrix_length(b0,n);
        set_matrix_length(c0,n);
        clock_gettime(CLOCK_REALTIME, &time1);
        pt_cb_pthr(a0,b0,c0);
        clock_gettime(CLOCK_REALTIME, &time2);
        time_stamp[OPTION][x] = diff(time1,time2);
        printf("iter %d done\n", x);
    }
    
    /* enable this to try the experiment on a machine with 8+ cores, and don't
     forget to also change OPTIONS definition at top! */
    /*
     NUM_THREADS = 8;
     OPTION++;
     printf("OPTION %d: pt_cb_pthr() with %d threads\n", OPTION, NUM_THREADS);
     for (i = 0; i < ITERS; i++) {
     init_matrix_rand(a0,BASE+(i+1)*DELTA);
     set_matrix_length(a0,BASE+(i+1)*DELTA);
     set_matrix_length(b0,BASE+(i+1)*DELTA);
     set_matrix_length(c0,BASE+(i+1)*DELTA);
     clock_gettime(CLOCK_REALTIME, &time1);
     pt_cb_pthr(a0,b0,c0);
     clock_gettime(CLOCK_REALTIME, &time2);
     time_stamp[OPTION][i] = diff(time1,time2);
     printf("iter %d done\n", i);
     }
     */
    
    printf("\n");
    printf("length, other data:\n");
    for (i = 0; i < ITERS; i++) {
        printf("%d, ", BASE+(i+1)*DELTA);
        for (j = 0; j < OPTIONS; j++) {
            if (j != 0) printf(", ");
            printf("%ld", (long int)((double)(CPG)*(double)
                                     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
        }
        printf("\n");
    }
    
    printf("\n Good Bye World!\n");
    
}/* end main */

/**********************************************/
/* Create matrix of specified length */
matrix_ptr new_matrix(long int len)
{
    long int i;
    
    /* Allocate and declare header structure */
    matrix_ptr result = (matrix_ptr) malloc(sizeof(matrix_rec));
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

/* Set length of matrix */
int set_matrix_length(matrix_ptr m, long int index)
{
    m->len = index;
    return 1;
}

/* Return length of matrix */
long int get_matrix_length(matrix_ptr m)
{
    return m->len;
}

/* initialize matrix to consecutive numbers starting with 0 */
int init_matrix(matrix_ptr m, long int len)
{
    long int i;
    
    if (len > 0) {
        m->len = len;
        for (i = 0; i < len*len; i++)
            m->data[i] = (data_t)(i);
        return 1;
    }
    else return 0;
}

/* initialize matrix to random values in [INIT_LOW, INIT_HIGH] */
int init_matrix_rand(matrix_ptr m, long int len)
{
    long int i;
    double fRand(double fMin, double fMax);
    
    if (len > 0) {
        m->len = len;
        for (i = 0; i < len*len; i++)
            m->data[i] = (data_t)(fRand((double)(INIT_LOW),(double)(INIT_HIGH)));
        return 1;
    }
    else return 0;
}

/* initialize matrix to random values bounded by a "gradient" envelope */
int init_matrix_rand_grad(matrix_ptr m, long int len)
{
    long int i;
    double fRand(double fMin, double fMax);
    
    if (len > 0) {
        m->len = len;
        for (i = 0; i < len*len; i++)
            m->data[i] = (data_t)(fRand((double)(0.0),(double)(i)));
        return 1;
    }
    else return 0;
}

/* initialize matrix */
int zero_matrix(matrix_ptr m, long int len)
{
    long int i,j;
    
    if (len > 0) {
        m->len = len;
        for (i = 0; i < len*len; i++)
            m->data[i] = (data_t)(IDENT);
        return 1;
    }
    else return 0;
}

data_t *get_matrix_start(matrix_ptr m)
{
    return m->data;
}

/* print matrix */
int print_matrix(matrix_ptr v)
{
    long int i, j, len;
    
    len = v->len;
    for (i = 0; i < len; i++) {
        printf("\n");
        for (j = 0; j < len; j++)
            printf("%.4f ", (data_t)(v->data[i*len+j]));
    }
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

double fRand(double fMin, double fMax)
{
    double f = (double)random() / (double)(RAND_MAX);
    return fMin + f * (fMax - fMin);
}


/***************************************************************************/
/* CPU bound multithreaded code. Here we use pthreads to do the same thing */
/* as pt_cb_base().  first, the worker thread function                       */
void *cb_work(void *threadarg)
{
    long int i, j, k, low, high;
    int done = 0;
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    int taskid = my_data->thread_id;
    matrix_ptr a0 = my_data->a;
    long int length = get_matrix_length(a0);
    data_t *aM = get_matrix_start(a0);
    long int n = get_matrix_length(a0);
    
    low = 1 + (taskid * length/NUM_THREADS);//(taskid * length * length)/NUM_THREADS;
    high = low + length/NUM_THREADS - 1;//((taskid+1)* length * length)/NUM_THREADS;
    
    double temp, mydiff = 0;
    
    while (!done){
        mydiff = 0;
        differ = 0;
        for (i = low ;i< high; i++){
            for (j = 1; j<n-1; j++){
                temp = aM[i*length+j];
                aM[i*length+j] = 0.2 * (aM[i*length+j] + aM[(i-1)*length+j] + aM[(i+1)*length+j] + aM[i*length+j+1] + aM[i*length+j-1]);
                mydiff += fabs(aM[i*length+j] - temp);
            }
        }
        
        pthread_mutex_lock(&diff_lock);
        differ+=mydiff;
        pthread_mutex_unlock(&diff_lock);
        pthread_barrier_wait(&bar1);
        
        if(differ/((n-2)*(n-2))<TOL){
            done = 1;
        }
        
        pthread_barrier_wait(&bar1);
    }
    
    pthread_exit(NULL);
}

/* Now, the pthread calling function */
void pt_cb_pthr(matrix_ptr a, matrix_ptr b, matrix_ptr c)
{
    long int i, j, k;
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int rc;
    long t;
    
    pthread_barrier_init(&bar1, NULL, NUM_THREADS);
    if (pthread_mutex_init(&diff_lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
    }
    
    
    for (t = 0; t < NUM_THREADS; t++) {
        thread_data_array[t].thread_id = t;
        thread_data_array[t].a = a;
        rc = pthread_create(&threads[t], NULL, cb_work,
                            (void*) &thread_data_array[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    for (t = 0; t < NUM_THREADS; t++) {
        if (pthread_join(threads[t],NULL)){
            printf("ERROR; code on return from join is %d\n", rc);
            exit(-1);
        }
    }
    
    pthread_barrier_destroy(&bar1);
    pthread_mutex_destroy(&diff_lock);
    
}
