#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
        printf(".\n");
    return c;
}

char my_get(char c){
    return (char)fgetc(stdin);
}

void map(char *array, int array_length, char (*f) (char)){

    for(int i=0; i<array_length; i++)
        array[i] = (*f)(array[i]);
}

int main(int argc, char **argv){

    char arr[5];
    map(arr,5,my_get);
    map(arr,5,cprt);
    map(arr,5,encrypt);
    map(arr,5,cprt);
    map(arr,5,dprt);
    map(arr,5,decrypt);
    map(arr,5,cprt);
    map(arr,5,dprt);

	return 0;
}


