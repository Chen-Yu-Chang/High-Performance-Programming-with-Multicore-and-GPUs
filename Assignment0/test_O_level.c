/**********************************************************************
 
 gcc -O0 test_O_level.c -o test_O_level
 gcc -O1 test_O_level.c -o test_O_level
 
 gcc -O0 -S test_O_level.c
 gcc -O1 -S test_O_level.c
 
 */

#include <stdio.h>

/*********************************************************************/
int main(int argc, char *argv[])
{
    long long int i, j, k, steps = 0;
    
    printf("\n Starting a loop \n");
    
    for (i = 0; i <= 100000000; i++) steps = steps + 3;
    
    printf("\n done \n");
}
