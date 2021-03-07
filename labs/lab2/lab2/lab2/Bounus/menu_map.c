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
    return (char)fgetc(stdin);
}

void map(char *array, int array_length, char (*f) (char)){
    for(int i=0; i<array_length; i++)
        array[i] = (*f)(array[i]);
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

    char empty_array[] = {'\0','\0','\0','\0','\0'};
    char* carray = empty_array;
    int carray_length = 5;

    int bonus = 2 ;
    
    /* functions pointers declarations */
    char  (*censorPtr)  (char) = &censor;
    char  (*encryptPtr) (char) = &encrypt;
    char  (*decryptPtr) (char) = &decrypt;
    char  (*dprtPtr)    (char) = &dprt;
    char  (*cprtPtr)    (char) = &cprt;
    char  (*my_getPtr)  (char) = &my_get;
    char  (*quitPtr)    (char) = &quit;
    char  (*junkPtr)    (char) = &bonus;

    struct fun_desc menu[] = { 
                               { "Censor", censorPtr },
                               { "Encrypt", encryptPtr },
                               { "Decrypt", decryptPtr },
                               { "Print dec", dprtPtr },
                               { "Print string", cprtPtr },
                               { "Get string", my_getPtr },
                               { "Quit", quitPtr },
                               { "Junk", junkPtr },
                               { NULL, NULL }
                             };

    
    char choice[1024]; 
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

    map(carray, carray_length, menu[index].fun);

    printf("\nDONE.\n");

    }
    return 0;
}