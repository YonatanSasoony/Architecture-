#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));

  char* iterMap = mapped_array;
  char* iterArr = array;

  for(int i=0; i<array_length; i++){
      *iterMap = (*f)(*iterArr);
      iterMap += 1 ;
      iterArr += 1 ;
  }

  return mapped_array;
}
 
int main(int argc, char **argv){

  char arr1[] =  {'H','!','E','Y','!'};
  char* arr2 = map(arr1, 5, censor);
  printf("the mapped array is:\t%s\n", arr2); 
  free(arr2);

}
