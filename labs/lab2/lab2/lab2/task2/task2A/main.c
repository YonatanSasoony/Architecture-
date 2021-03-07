#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}
 
void map(char *array, int array_length, char (*f) (char)){

    for(int i=0; i<array_length; i++)
        array[i] = (*f)(array[i]);
}
 
int main(int argc, char **argv){

  char arr1[] =  {'!','H','!','E','Y','!'};
  map(arr1, 6, censor);
  printf("the mapped array is:\t%s\n", arr1); 

}
