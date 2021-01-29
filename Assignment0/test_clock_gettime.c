/****************************************************************************/

/* To compile on the lab machines:
 *
 *     gcc -O0 test_clock_gettime.c -lm -lrt -o test_clock_gettime
 *
 * Note that other machines, like your laptop, might not need -lrt option.
 * Also, the -lm option is only needed if you use a math library routine,
 * like sin() as suggested below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define SIZE 10000000

// As described in the clock_gettime manpage (type "man clock_gettime" at the
// shell prompt), a "timespec" is a structure that looks like this:
//
//       struct timespec {
//         time_t   tv_sec;   /* seconds */
//         long     tv_nsec;  /* and nanoseconds */
//       };

int main(int argc, char *argv[])
{
    struct timespec diff(struct timespec start, struct timespec end);
    struct timespec time1, time2;
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    long long int i, j, k;
    long long int time_sec, time_ns;
    
    i = j = k = 0;
    
    // get start time
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    
    
    
    // Here you should ADD YOUR OWN CODE and modify it until it takes about
    // one second to run. For example, figure out how many times the computer
    // can execute the sin(x) function inside a loop, where x changes each
    // time sin(x) is called, add up the values of sin(x). */
    
    
    
    // get end time
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    
    
    // ADD CODE HERE to print something you computed in between the two calls
    // to clock_gettime, such as a sum. (to understand why, refer to the
    // test_O_level.c part of the assignment)
    
    
    
    // compute elapsed time and print.
    time_sec = 0; // MODIFY: Replace "0" with a call to "diff()" function below
    time_ns  = 0; // MODIFY: Replace "0" with another call to "diff()" function below
    
    printf("that took %ld sec and %ld nsec (%ld.%09ld sec)\n", time_sec, time_ns, time_sec, time_ns);
    
} /* end main() */


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
