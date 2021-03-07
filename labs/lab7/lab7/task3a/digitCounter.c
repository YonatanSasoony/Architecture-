#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv){
    digit_cnt(argv[1]);
}

int digit_cnt(char* str){
    int counter = 0;
    for(int i=0; str[i] != 0; i++)
        if( (str[i] >= 48)  &&  (str[i]<= 57) )
            counter++;
    return counter;
}