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
    virus *v = (virus*) malloc(sizeof(virus));
    char N[2] ;
    fread(N,1,2,file);
    v->SigSize = N[1] * 256 + N[0];
    fread(v->virusName,1,16,file);
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
  return 0;
}








