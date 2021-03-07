#include<stdio.h>

int main (int argc, char **argv){
    int c = 0;
    while( (c=fgetc(stdin)) != EOF){ 
        if(c>=65 && c<=90)
            c = c + 32 ;
        fputc(c , stdout);
    }
    printf("\n");
    return 0;
}