/**********************************
 Data import/export demo code
 (by Daniel Collins, EC 500 Spring 2011)
 **********************************/

#include <stdio.h>
#include <stdlib.h>

/* Open-Office-Compatible printout */
void print_ooc()
{
    long long int i,j,k,l;
    
    for(j=0;j<9;j++) {
        printf("col%lld",j);
        j==8 ? printf(" ") : printf(",");
    }
    for(i=0;i<10;i++)
        for(j=0;j<i;j++) {
            //prints new line if new row, else prints separating comma
            j==0 ? printf("\n") : printf(",");
            k=i*j;
            printf("%lld",k);
        }
    
    printf("\n\n ,");
    for(j=0;j<9;j++) {
        printf("col%lld",j);
        j==8 ? printf(" ") : printf(",");
    }
    for(i=0;i<10;i++)
        for(j=0;j<i;j++) {
            //prints new line if new row, else prints separating comma
            j==0 ? printf("\n%lld,",i*10) : printf(",");
            k=i*j;
            printf("%lld",k);
        }
    
    printf("\n\n ,");
    for(j=0;j<9;j++) {
        printf("col%lld",j);
        j==8 ? printf(" ") : printf(",");
    }
    l=1;
    for(i=0;i<10;i++) {
        l*=10;
        for(j=0;j<i;j++) {
            //prints new line if new row, else prints separating comma
            j==0 ? printf("\n%lld,",l) : printf(",");
            k=i*j;
            printf("%lld",k);
        }
    }
    
}

/* MATLAB-compatible printout */
void print_matlab()
{
    int i,j,k;
    
    for(i=0;i<10;i++) {
        for(j=0;j<i;j++) {
            //prints new line if new row, else prints separating comma
            j==0 ? printf("\n") : printf(",");
            k=i*j;
            printf("%d",k);
        }
        
        //Pads rows for MATLAB
        for(j=i;j<10;j++) {
            //prints new line if new row, else prints separating comma
            j==0 ? printf("\n") : printf(",");
            printf("NaN");
        }
    }
}

int main()
{
    print_ooc();
    //print_matlab();
    printf("\n");
    return 0;
}
