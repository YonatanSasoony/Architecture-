#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***************************** functions from task 2 **************************/

char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}

char encrypt(char c){
    if( (c < 0x20) || (c > 0x7E) ) 
        return c;
    return c+3;
}

char decrypt(char c){
    if( (c < 0x20) || (c > 0x7E) ) 
        return c;
    return c-3;
}

char dprt(char c){
    printf("%d\n", c);
    return c;
}

char cprt(char c){
    if(( c >= 0x20) && (c <= 0x7E) )
        printf("%c\n",c);
    else
        printf(".");
    return c;
}

char my_get(char c){
    return fgetc(stdin);
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

char quit(char c){
    if( c == 'q')
        exit(0);
    return c;
}
/************************************************************************/

struct fun_desc {
  char *name;
  char (*fun)(char);
};


int main(int argc, char** argv){

    char* carray = (char*)malloc(5*sizeof(char));
    for(int i=0; i<5; i++)
        carray[i] = '\0';
    
    /* functions pointers declarations */
    char  (*censorPtr)  (char) = &censor;
    char  (*encryptPtr) (char) = &encrypt;
    char  (*decryptPtr) (char) = &decrypt;
    char  (*dprtPtr)    (char) = &dprt;
    char  (*cprtPtr)    (char) = &cprt;
    char  (*my_getPtr)  (char) = &my_get;
    char  (*quitPtr)    (char) = &quit;

    struct fun_desc menu[] = { 
                               { "Censor", censorPtr },
                               { "Encrypt", encryptPtr },
                               { "Decrypt", decryptPtr },
                               { "Print dec", dprtPtr },
                               { "Print string", cprtPtr },
                               { "Get string", my_getPtr },
                               { "Quit", quitPtr },
                               { NULL, NULL }
                             };

    
    char choice[256];
    int index = 0;
    int menuLen = (sizeof(menu) / 16) - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;     

    while(1){

    printf("Please choose a function:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d) %s\n", i,menu[i].name);

    scanf("%s", choice);
    fgetc(stdin);// for the \n after the user make his choice
    index = atoi(choice);

    if( index >= lowBound   &&   index <= highBound )
        printf("Within bounds\n");
    else{
        printf("Not within bounds\n");
        exit(0);
    }

    char* temp = map(carray,5, menu[index].fun);
    free(carray);
    carray = temp; 

    printf("\nDONE.\n");

    }
    return 0;
}