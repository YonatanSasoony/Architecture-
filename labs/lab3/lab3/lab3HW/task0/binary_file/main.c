
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void PrintHex(char* buffer, int length){
    for (int i=0; i<length; i++)
        printf("%02X ",buffer[i] & 0x000000ff); 
}

int main(int argc, char** argv){

    FILE* fileName = fopen(argv[1],"r");
    fseek(fileName, 0, SEEK_END);
    int fileSize = ftell(fileName);
    fseek(fileName, 0, SEEK_SET);

    char* buffer = (char*) malloc (sizeof(char)*fileSize);
    if(buffer == NULL) printf("out of memory error\n"); 
    
    int couldRead = fread(buffer, 1, fileSize, fileName);
    if(couldRead != fileSize) printf("reading error\n");
    
    PrintHex(buffer, fileSize);

    fclose(fileName);
    free(buffer);

    printf("\n");
    return 0;
}
