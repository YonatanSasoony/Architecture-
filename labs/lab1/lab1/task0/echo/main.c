#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  char echo [256];
  int c = 0;
  int i = 0;
  
  printf("write something\n");
  fflush(stdout);
  
  while((c=fgetc(stdin))!='\n')
    echo[i++] = (char) c ; 
  echo[i] = 0;

  fprintf(stdout, "%s\n", echo);

  return 0;

}