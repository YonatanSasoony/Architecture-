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

char quit(char c){
    if( c == 'q')
        exit(0);
    return c;
}

int main(int argc, char** argv){

int base_len = 5;
char arr1[base_len];
map(arr1, base_len, my_get);
map(arr1, base_len, encrypt);
map(arr1, base_len, dprt);
map(arr1, base_len, decrypt);
map(arr1, base_len, cprt); 

}