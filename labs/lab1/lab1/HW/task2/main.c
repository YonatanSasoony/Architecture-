#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define lowerCaseA 97
#define lowerCaseZ 122

int main(int argc, char **argv) {

  int c = 0;  
  int isD = 0;
  int isE = 0;
  int plus = 0;  
  char *code = NULL;
  int len = 0;
  int index = 0 ;
  FILE * output = stdout;
  FILE * input = stdin;
  int isO = 0 ;
  int isI = 0 ;

  for (int i=1; i<=argc; i++){
    if(argv[i] != NULL && strncmp(argv[i], "-D", 2) == 0) isD = 1;
    if(argv[i] != NULL && strncmp(argv[i], "-e", 2) == 0){
      isE = 1;
      code = argv[i] + 2 ;
      }  
    if(argv[i] != NULL && strncmp(argv[i], "+e", 2) == 0){ 
      isE = 1;
      plus = 1;
      code = argv[i] + 2 ;
      }  
    if(argv[i]!=NULL && strncmp(argv[i],"-o",2) == 0){
        output = fopen(argv[i] + 2, "w");
        isO = 1 ;
    }
    if(argv[i] != NULL && strncmp(argv[i],"-i", 2) == 0){
        input = fopen(argv[i] + 2, "r");    
        isI = 1;
        if(input == NULL){
          printf("ERROR : file cannot be opened for reading\n");
          return 0;
        }
    }

  }

    if(isE)
      while(code[len] != '\0')
          len++;

do{
    printf("write something\n");
    fflush(stdout);

    while((c=fgetc(input))!='\n'){  
        
        if(feof(input) != 0) break;//for avoiding infinite loop 

        if(isD == 1)
          fprintf(stderr, "%i\t",c);

        if(isE == 1){
          if(plus == 1)
            c = c + (code[index % len] - '0') ;
          else 
            c = c - (code[index % len] - '0') ;
          index++;   
        }else // isE == 0
           if(c >= lowerCaseA && c <= lowerCaseZ)
                c=c-32;

        if(isD == 1)
          fprintf(stderr, "%i\n",c);

        fputc(c , output);  
        }
        
        fputc('\n' , output); 

} while (feof(input) == 0);

    if(isO)
        fclose(output);
    else 
        fflush(output);

    if(isI) 
        fclose(input);
        
    printf("\n");  
    index = 0;

  return 0;
  }

  
  
