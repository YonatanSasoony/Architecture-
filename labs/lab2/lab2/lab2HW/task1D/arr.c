#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main (int argc, char** argv){


    int iarray[] = {1,2,3};
    char carray[] = {'a','b','c'};
    
    int* iarrayPtr = iarray;
    char* carrayPtr = carray;

    printf("iarray:\t[\t");
    for(int i=0; i<3; i++)
        printf("%d\t", *(iarrayPtr+i));
    printf("]\n");

    printf("\ncarray:\t[\t");
    for(int i=0; i<3; i++)
        printf("'%c'\t", *(carrayPtr+i) );
    printf("]\n\n");


    void* p;
    printf("pAddress:\t%p\n\n", p);

    return 0;

}
