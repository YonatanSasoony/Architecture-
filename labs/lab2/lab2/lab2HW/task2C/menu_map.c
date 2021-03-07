
//char quit(char c); /* Gets a char c,  and if the char is 'q' , 
//ends the program with exit code 0. Otherwise returns c. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char quit(char c){
    if( c == 'q')
        exit(0);
    return c;
}

int main(int argc, char** argv){
    printf("insert a char:\n");
    char c = fgetc(stdin);
    char retVal = quit(c);
    printf("%c\n\n",retVal);
    return 0;
}