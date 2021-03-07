
#include <stdio.h>
#include <string.h>

int main (int argc, char **argv){
    int c = 0;
    int isD = 0;

    for(int i=1; i<argc; i++){
        if( (strncmp("-D", argv[i],2)==0) )
            isD = 1 ;
        else {
            printf("invalid parameter - %s\n" , argv[i]);
            return 1;
        }
    }

    while( (c=fgetc(stdin)) != EOF){

        if(isD == 1)
            fprintf(stderr, "%d\t",c);

        if(c>=65 && c<=90)
            c = c + 32 ;

        if(isD == 1)
            fprintf(stderr, "%d\n", c);

        fputc(c , stdout);
    }

    printf("\n");
    return 0;
}