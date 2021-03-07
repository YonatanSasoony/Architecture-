#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 256

char* inputFileName;

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    char* sig;
} virus;

typedef struct link link;
 
struct link {
    link *nextVirus;
    virus *vir;
};

void PrintHex(char* buffer, int length, FILE* output){
    for (int i=0; i<length; i++)
        fprintf(output, "%02X ",buffer[i] & 0x000000ff); 
}

virus* readVirus(FILE* file){
    char buf[18];
    size_t ableToRead = fread(buf,1,18,file);

    if(ableToRead == 0) return NULL; //no more info for reading the next virus

    virus *v = (virus*) malloc(sizeof(virus));

    v->SigSize = buf[1] * 256 + buf[0]; //256=16^2 for little endian

    sscanf(buf + 2, "%s", v->virusName);

    v->sig = (char*) malloc(v->SigSize*sizeof(char));
    fread(v->sig,1,v->SigSize,file);

    return v;
}

void printVirus(virus* virus, FILE* output){
    if(virus == NULL) return ;
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

void list_print(link *virus_list, FILE* output){
    if (virus_list != NULL){   
        printVirus(virus_list->vir, output); 
        list_print(virus_list->nextVirus, output);
    }
}
     
link* list_append(link* virus_list, virus* data){  

    link* newLink = (link *) malloc (sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = NULL;

    if(virus_list == NULL)
        return newLink;

    link* iter = virus_list;
    while(iter->nextVirus != NULL)
        iter = iter->nextVirus;
    iter->nextVirus = newLink;

    return virus_list;
}

void list_free(link *virus_list){
    if(virus_list != NULL){
        list_free(virus_list->nextVirus);
        destroyVirus(virus_list->vir);
        free(virus_list);
    }
}

link* loadSig (link* virusList){
    if(virusList != NULL) list_free(virusList);
    char fileName[MAX];
    char name[MAX];
    printf("enter a file name:\n");
    fgets(fileName, MAX, stdin);
    sscanf(fileName, "%s", name);
    FILE* input = fopen(name,"r");
    if(input == NULL){
        printf("file was not found\n");
        return virusList;
    }
    link* head = NULL; 
    virus* nextVirus = NULL;
    while(1){
        nextVirus = readVirus(input);
        if(nextVirus == NULL) // could not read the next virus
            break;
        else 
            head = list_append(head, nextVirus);
    }
    fclose(input);
    return head; 
}

link* printSig (link* virusList){
    if (virusList != NULL)
        list_print(virusList ,stdout);
    return virusList;
}

void printVirusInfo (int start, char* name, int size){
    printf("The starting byte location in the suspected file: %d\nThe virus name: %s\nThe size of the virus signature: %d\n\n", start,name,size);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link* iter = virus_list;
    while(iter != NULL){
        int virusSize = iter->vir->SigSize;
        char* virusSig = iter->vir ->sig;
        for(int i=0; i<=size-virusSize; i++)
            if(memcmp(virusSig, buffer + i, virusSize) == 0)
                printVirusInfo(i, iter->vir->virusName, virusSize);             
        iter = iter->nextVirus;
    }
}

int min (int a, int b){if(a<b) return a; return b;}

link* detectViruses (link* virusList){

    FILE* file = fopen(inputFileName,"r");
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char buffer[10000];
    fread(buffer,1,fileSize,file);

    detect_virus(buffer, min(fileSize,10000) , virusList);

    fclose(file);
    return virusList;

}

link* quit (link* virusList){
    list_free(virusList);
    exit(0);
    return virusList;
}

typedef struct MENU {
  char *name;
  link* (*fun)(link*);
}MENU;

int main(int argc, char** argv){

    struct MENU menu[] = { { "Load signatures", loadSig}, { "Print signatures", printSig}, 
                           { "Detect viruses", detectViruses}, { "Quit", quit}, { NULL, NULL }  };
    
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    

    link* myLink = NULL;

    inputFileName = argv[1]; 

    while(1){

    printf("Please choose a function:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d) %s\n", i+1,menu[i].name);

    fgets(choice,MAX,stdin);
    index = atoi(choice) - 1 ;

    if( index < lowBound   ||   index > highBound ){
        printf("Not within bounds\n");
        exit(0);
    }

    myLink = menu[index].fun(myLink);

    printf("\n");

    }
    return 0;
}