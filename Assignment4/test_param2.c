/*************************************************************************
 
 gcc -pthread test_param2.c -o test_param2
 
 */

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define NUM_THREADS 10

int a=0;
/********************/
void *work(void *i)
{
    long int j, k;
    //int f = *((int*)(i));  // get the value being pointed to
    int *g = (int*)(i);    // get the pointer itself
    
    /* busy work */
    k = 0;
    for (j=0; j < 10000000; j++) {
        k += j;
    }
    
    printf("Hello World from %lu with value %d\n", pthread_self(), g[a]);
    g[a] = g[a] * 5;
    a = a + 1;
    //printf("in work(): f=%2d, k=%ld, *g=%d\n", f, k, *g);
    
    pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
    int i;
    long j, k;
    pthread_t id[NUM_THREADS];
    int test[NUM_THREADS]={0,1,2,3,4,5,6,7,8,9};
    
    for (i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&id[i], NULL, work, (void *)(&test))) {
            printf("ERROR creating the thread\n");
            exit(19);
        }
    }
    
    /* busy work */
    k = 0;
    for (j=0; j < 100000000; j++) {
        k += j;
    }
    for (k=0;k<NUM_THREADS;k++){
        printf("Test: %d\n", test[k]);
    }
    
    printf("k=%ld\n", k);
    printf("After creating the threads.  My id is %lx, i = %d\n",
           (long)pthread_self(), i);
    
    return(0);
    
} /* end main */
