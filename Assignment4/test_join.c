/*************************************************************************
 
 Simple thread create and exit test
 
 gcc -pthread test_join.c -o test_join
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

/*************************************************************************/
void *work(void *i)
{
    sleep(3);
    printf(" Hello World!  It's me, thread #%lx --\n", (long)pthread_self());
    pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
    int i;
    pthread_t id[NUM_THREADS];
    
    printf("Hello test_join.c\n");
    
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&id[i], NULL, work, NULL)) {
            exit(19);
        }
    }
    
    printf("main() -- After creating the thread.  My id is: %lx\n",
           (long)pthread_self());
    
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(id[i], NULL)) {
            exit(19);
        }
    }
    
    printf("After joining, Good Bye World!\n");
    
    return(0);
} /* end main */
