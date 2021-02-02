/*****************************************************************************
 To compile:
 
 gcc -O1 test_combine2d.c -lrt -o test_combine2d
 
 On some machines you might not need to link the realtime library. If
 you get an error like "library not found for -lrt", do not use the
 -lrt option.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.0           /* Cycles per GHz -- Adjust to your computer */

#define BASE  1000
#define ITERS 2
#define DELTA 5000

#define OPTIONS 2
#define IDENT 0.0
#define OP +

typedef double data_t;

/* Create abstract data type for allocated array */
typedef struct {
    long int row_len;
    data_t *data;
} array_rec, *array_ptr;

/*****************************************************************************/
int main(int argc, char *argv[])
{
    int OPTION;
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    struct timespec time_stamp[OPTIONS][ITERS+1];
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    array_ptr new_array(long int row_len);
    int set_row_length(array_ptr v, long int row_len);
    long int get_row_length(array_ptr v);
    int init_array(array_ptr v, long int row_len);
    data_t *data_holder;
    void combine2D(array_ptr v, data_t *dest);
    void combine2D_rev(array_ptr v, data_t *dest);
    
    long int i, j, k;
    long int time_sec, time_ns;
    long int MAXSIZE = BASE+(ITERS+1)*DELTA;
    
    printf("2D Combine tests\n\n");
    
    /* declare and initialize the array structure */
    array_ptr v0 = new_array(MAXSIZE);
    data_holder = (data_t *) malloc(sizeof(data_t));
    init_array(v0, MAXSIZE);
    
    OPTION = 0;
    for (i = 0; i < ITERS; i++) {
        set_row_length(v0, BASE+(i+1)*DELTA);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        combine2D(v0, data_holder);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][i] = diff(time1,time2);
    }
    
    OPTION++;
    for (i = 0; i < ITERS; i++) {
        set_row_length(v0, BASE+(i+1)*DELTA);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        combine2D_rev(v0, data_holder);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        time_stamp[OPTION][i] = diff(time1,time2);
    }
    
    printf("row_len, combine2D, combine2D_rev\n");
    for (i = 0; i < ITERS; i++) {
        printf("%ld, ", BASE+(i+1)*DELTA);
        for (j = 0; j < OPTIONS; j++) {
            if (j != 0) printf(", ");
            printf("%ld", (long int)((double)(CPG)*(double)
                                     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
        }
        printf("\n");
    }
    
    printf("\n");
    
}/* end main */
/*********************************/

/* Create 2D array of specified size-per-dimension (row_len parameter is
 both the row length and the number of rows) */
array_ptr new_array(long int row_len)
{
    long int i;
    
    /* Allocate and declare header structure */
    array_ptr result = (array_ptr) malloc(sizeof(array_rec));
    if (!result) return NULL;  /* Couldn't allocate storage */
    result->row_len = row_len;
    
    /* Allocate and declare array */
    if (row_len > 0) {
        data_t *data = (data_t *) calloc(row_len*row_len, sizeof(data_t));
        if (!data) {
            free((void *) result);
            printf("\n COULDN'T ALLOCATE %ld BYTES STORAGE \n",
                   row_len*row_len * sizeof(data_t));
            return NULL;  /* Couldn't allocate storage */
        }
        result->data = data;
    }
    else result->data = NULL;
    
    return result;
}

/* Set row-length of array (must already be allocated with new_array and
 size used when allocating must be big enough) */
int set_row_length(array_ptr v, long int row_len)
{
    v->row_len = row_len;
    return 1;
}

/* Return row length of array (which is also the number of columns) */
long int get_row_length(array_ptr v)
{
    return v->row_len;
}

/* initialize 2D array */
int init_array(array_ptr v, long int row_len)
{
    long int i;
    
    if (row_len > 0) {
        v->row_len = row_len;
        for (i = 0; i < row_len*row_len; i++) v->data[i] = (data_t)(i);
        return 1;
    }
    else return 0;
}

data_t *get_array_start(array_ptr v)
{
    return v->data;
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

/************************************/

/* Combine2D: Use operator OP to add or multiply all elements in the array.
 Accumulate the result in a local variable */
void combine2D(array_ptr v, data_t *dest)
{
    long int i, j;
    long int get_row_length(array_ptr v);
    data_t *get_array_start(array_ptr v);
    long int length = get_row_length(v);
    data_t *data = get_array_start(v);
    data_t acc = IDENT;
    
    for (i = 0; i < length; i++)
        for (j = 0; j < length; j++)
            acc = acc OP data[i*length+j];
    *dest = acc;
}

/* Combine2D_rev:  Like combine2D but the loops are interchanged. */
void combine2D_rev(array_ptr v, data_t *dest)
{
    long int i, j;
    long int get_row_length(array_ptr v);
    data_t *get_array_start(array_ptr v);
    long int length = get_row_length(v);
    data_t *data = get_array_start(v);
    data_t acc = IDENT;
    
    for (j = 0; j < length; j++)
        for (i = 0; i < length; i++)
            acc = acc OP data[i*length+j];
    *dest = acc;
}
