// gcc -O1 stream_simple.c -lm -o stream_simple

# include <stdio.h>
# include <math.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>


#define ITERS 1000        // number of timing sample to average over
#define MAXSIZE 1000000   // run loop 1M times


#define FLOPs_per_Loop 32.0            //Modify ALL lines marked "Modify"
#define Unique_Reads_per_Loop 1.0     //Modify ALL lines marked "Modify"
#define Arithmetic_Intensity FLOPs_per_Loop/Unique_Reads_per_Loop   // recommended range =  [1/8 , 2]

extern double mysecond();                // get current time in seconds
static float a[MAXSIZE],b[MAXSIZE];     // input and output arrays
int integer[7][MAXSIZE];                // integer values to vary the number of memory accesses without affecting FLOPs. Current maximum is 8 reads/flop.

int main()
{
    double times [ITERS];
    long int i, j,k;
    double time = 0;
    
    for(i=0; i<MAXSIZE; i++) {
        a[i] = (float) (i+1);
        b[i] = (float) (2*i);
    }
    for (i=0; i< ITERS; i++){
        times[i] = mysecond();
        for (j=0; j<MAXSIZE; j++){
            b[j] = a[j] * a[j] * a[j] * a[j] * a[j] * a[j] * a[j] * a[j] * a[j]; //Modify ALL lines marked "Modify"
        }
        times[i] = mysecond() - times[i];
    }
    
    for (i=0;i<ITERS;i++){
        time = time + times[i];
    }
    time = time/ (double)ITERS;
    printf ("AI = %f    GFLOPs/s = %f \n",
            Arithmetic_Intensity, (double)(Arithmetic_Intensity)/(time*1000.0));
    
    return 0;
}

double mysecond()
{
    struct timeval tp;
    struct timezone tzp;
    int i;
    
    i = gettimeofday(&tp,&tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}
