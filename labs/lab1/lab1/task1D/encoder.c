
#include <stdio.h>
#include <string.h>

int main (int argc, char **argv){
    int c = 0;
    int isD = 0;
    int isE = 0;
    int plus = 0;
    char *code = NULL;
    int len = 0;
    int index = 0;
    int isO = 0;
    FILE* output = stdout;

    for(int i=1; i<argc; i++){
        if( (strncmp("-D", argv[i],2)==0) )
            isD = 1 ;
        else if ((strncmp("+e", argv[i],2)==0)){
            isE = 1;
            plus = 1;
            code = argv[i] + 2 ;
        }
        else if ((strncmp("-e", argv[i],2)==0)){
            isE = 1;
            code = argv[i] + 2 ;
        }
        else if((strncmp("-o", argv[i],2)==0)){
            isO = 1;
            output = fopen(argv[i] + 2, "w") ;
        }
        else {
            printf("invalid parameter - %s\n" , argv[i]);
            return 1;
        }
    }


    if(isE)
        while(code[len] != '\0')
            len++;

    while( (c=fgetc(stdin)) != EOF ){

        if(c == '\n'){
            fputc (c , output);
            index = 0;
        }
            
        else{ if(isD == 1)
                fprintf(stderr, "%d\t",c);

              if(isE == 1){
                  if(plus == 1)
                      c = c + ( code[index] - '@' ) ;
                  else 
                      c = c - ( code[index] - '@' ) ;

                  index = (index + 1) % len ;
              }
              else if(c>=65 && c<=90)
                      c = c + 32 ;

              if(isD == 1)
                  fprintf(stderr, "%d\n", c);

              fputc(c , output);
        }
    }
    if(isO == 1)
        fclose (output);
    printf("\n");
    return 0;
}