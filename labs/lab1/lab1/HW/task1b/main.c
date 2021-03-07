#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {

  int c = 0;  
  int isD = 0;  

  for (int i=1; i<=argc; i++){
    if(argv[i] != NULL && strncmp("-D", argv[i],2) == 0)
      isD = 1;
  }
      
  printf("write something\n");
  fflush(stdout);
  
  while((c=fgetc(stdin))!='\n'){  
      if(isD == 1)
        fprintf(stderr, "%i\t",c);
      if(c >= 97 && c <= 122)
        c=c-32;
      if(isD == 1)
        fprintf(stderr, "%i\n",c);

      fputc(c , stdout);  
      }

  fflush(stdout);
  printf("\n");  
  
  return 0;
  }

  
  
