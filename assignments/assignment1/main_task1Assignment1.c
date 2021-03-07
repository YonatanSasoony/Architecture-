
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX 256

extern void assFunc(int x,int y);


char c_checkValidity(int x, int y){
    if (x>=y) 
        return 1;
    else
        return 0;
}

int main(int argc, char**argv){

    int x, y;
    char buffer[MAX];
    fgets(buffer, MAX, stdin);
    sscanf(buffer,"%d",&x);
     fgets(buffer, MAX, stdin);
    sscanf(buffer,"%d",&y);
    assFunc(x, y);

    return 0;
}

