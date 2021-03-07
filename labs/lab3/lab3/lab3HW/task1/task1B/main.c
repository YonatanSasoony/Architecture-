#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 30

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

/* Print the data of every link in list to the given stream. 
Each item followed by a newline character. */
void list_print(link *virus_list, FILE* output){
    if (virus_list != NULL){   
        list_print(virus_list->nextVirus, output);
        printVirus(virus_list->vir, output); 
    }
}
     
/* Add a new link with the given data to the list 
(either at the end or the beginning, depending on what your TA tells you),
and return a pointer to the list (i.e., the first link in the list).
If the list is null - create a new entry and return a pointer to the entry. */
link* list_append(link* virus_list, virus* data){
    link* newLink = (link *) malloc (sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;
}

link* helper (link* current, virus* data, link* head){
    if(current == NULL && current==head){
        link* current = (link *) malloc (sizeof(link));
        current->vir = data;
        current->nextVirus = NULL;
        return head;
    }
    else
        return helper(current->nextVirus, data, head);
}

link* list_append2(link* virus_list, virus* data){        
    return helper(virus_list,data,virus_list);
}

 /* Free the memory allocated by the list. */
void list_free(link *virus_list){
    if(virus_list != NULL){
        list_free(virus_list->nextVirus);
        destroyVirus(virus_list->vir);
        free(virus_list);
    }
}

link* freeHead (link* head){
    link* ret = head->nextVirus;

    destroyVirus(head->vir);
    free(head);

    return ret;
}

link* loadSig (link* virusList){
    char fileName[MAX];
    char name[MAX];
    printf("enter a file name:\n");
    // scanf("%s", fileName);
    // fgetc(stdin);
    fgets(fileName, MAX, stdin);
    sscanf(fileName, "%s", name);
    FILE* input = fopen(name,"r");
    if(input == NULL){
        printf("file was not found\n");
        return virusList;
    }
    link* head = NULL;
    while(!feof(input)){
        virus* nextVirus = readVirus(input);
        head = list_append(head, nextVirus);
    }
    fclose(input);
    return freeHead(head);
}

link* printSig (link* virusList){
    if (virusList != NULL)
        list_print(virusList ,stdout);
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

    struct MENU menu[] = { { "Load signatures", loadSig}, { "Print signatures", printSig}, { "Quit", quit}, { NULL, NULL }  };
    
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    

    link* myLink = NULL;

    while(1){

    printf("Please choose a function:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d) %s\n", i+1,menu[i].name);

    // scanf("%s", choice);
    // fgetc(stdin);
    // index = atoi(choice) - 1 ;
    fgets(choice,MAX,stdin);
    index = atoi(choice) - 1 ;

    if( index >= lowBound   &&   index <= highBound )
        printf("Within bounds\n");
    else{
        printf("Not within bounds\n");
        exit(0);
    }

    myLink = menu[index].fun(myLink);

    printf("\n");

    }
    return 0;
}









