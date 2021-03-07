#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  int c = 0;
  
  printf("write something\n");
  fflush(stdout);
  
  while((c=fgetc(stdin))!='\n'){
    if(c>=97 && c<=122)
        c = c - 32 ;
    fputc(c , stdout);
  }

  fflush(stdout);
  printf("\n");  
  
  return 0;

}