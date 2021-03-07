#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    char* sig;
} virus;

void PrintHex(char* buffer, int length, FILE* output){
    for (int i=0; i<length; i++)
        fprintf(output, "%02X ",buffer[i] & 0x000000ff); 
}

virus* readVirus(FILE* file){
    char buf[18];
    size_t ableToRead = fread(buf,1,18,file);

    if(ableToRead == 0)  return NULL; //no more info for reading the next virus

    virus *v = (virus*) malloc(sizeof(virus));

    v->SigSize = buf[1] * 256 + buf[0]; //256=2^8

    sscanf(buf + 2, "%s", v->virusName);

    v->sig = (char*) malloc(v->SigSize*sizeof(char));
    fread(v->sig,1,v->SigSize,file);

    return v;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n" , virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output,"signatue: \n" );
    PrintHex(virus->sig,virus->SigSize, output);
    fprintf(output, "\n\n");
}

void destroyVirus(virus* v){
    free(v->sig);
    free(v);
    v=NULL;
}

int main(int argc, char** argv){
  FILE* file = fopen(argv[1],"r");
  virus* nextVirus = NULL;
  for (int i=0; i<11; i++){
    nextVirus = readVirus(file);
    printVirus(nextVirus,stdout);
    destroyVirus(nextVirus);
  }
  fclose(file);
  return 0;
}








