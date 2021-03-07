#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#define MAX 1024
#define MAX_FILE_NAME_SIZE 100

typedef struct {
    char debug_mode; 
    char file_name[128];
    int curr_fd;
    struct stat fd_stat; /* this is needed to  the size of the file */
    Elf32_Ehdr *header; /* this will point to the header structure */
    void *map_start;/* will point to the start of the memory mapped file */
} state;

void toggle_debug(state* currState){
    if(currState->debug_mode){
        currState->debug_mode = 0;
        printf("Debug flag now off");
    }
    else{
        currState->debug_mode = 1;
        printf("Debug flag now on");        
    }
} 

void print_deubg(state* currState){
    if(currState->debug_mode)
        printf("file name:%s\ncurrent fd:%d\n",currState->file_name,currState->curr_fd);
} 

void print_header(Elf32_Ehdr* header,state* currState){
    printf("Magic:\t%c%c%c\n", header->e_ident[EI_MAG1],header->e_ident[EI_MAG2],header->e_ident[EI_MAG3]);
    if (header->e_ident[EI_DATA] == 1)
        printf("Data:\t %d-bit format, liitle endian\n",header->e_ident[EI_CLASS]*32 );//header->e_machine);
    else
        printf("Data:\t %d-bit format, big endian\n",header->e_ident[EI_CLASS]*32 );//header->e_machine);

    printf("Entry point address:\t0x%X\n", header->e_entry);
    printf("Section table header offset:\t%d\n", header->e_shoff);
    printf("Number of section headers:\t%d\n", header->e_shnum);
    printf("The size of each section header entry:%d\n",header->e_shentsize);        
    printf("Program header table offset:\t%d\n", header->e_phoff);
    printf("Number of program headers:\t%d\n", header->e_phnum);
    printf("The size of each program header entry:%d\n",header->e_phentsize);
}

void examine_elf (state* currState){
     

    printf("Enter a file name:\n");
    fgets(currState->file_name, MAX_FILE_NAME_SIZE, stdin);
    sscanf(currState->file_name,"%s",currState->file_name);
    if(currState->curr_fd != -1) 
        if(close(currState->curr_fd) < 0){
            perror("error in close");
            return;
        }

    if((currState->curr_fd = open(currState->file_name, O_RDONLY)) < 0){
        //if faild open's return val is -1
        perror("error in open");
        return;
    }

    if(fstat(currState->curr_fd, &currState->fd_stat) != 0 ) {
        if(close(currState->curr_fd) < 0)
            perror("error in close*");
        currState->curr_fd = - 1;
        perror("stat failed");
        return;
    }
    printf("%d\n",currState->curr_fd);
    if ( (currState->map_start = mmap(0, currState->fd_stat.st_size, PROT_READ, MAP_SHARED, currState->curr_fd, 0)) == MAP_FAILED ) {
        if(close(currState->curr_fd) < 0)
            perror("error in close*");
        currState->curr_fd = - 1;
        perror("mmap failed");
        return;
    }

    if(!strncmp( ((char*)(currState->map_start+1)),"ELF",3))
        currState->header = (Elf32_Ehdr *) currState->map_start;
    else{
        if(close(currState->curr_fd) < 0)
            perror("error in close*");
        currState->curr_fd = - 1;
        perror("not an ELF");
        munmap(currState->map_start, currState->fd_stat.st_size);
        return;
    }

    print_header(currState->header,currState);
}

void quit (state* currState){
    if(currState->debug_mode)
        printf("quitting\n");
    if(currState->curr_fd != -1){
        if(close(currState->curr_fd) < 0)
            perror("error in close*");
        munmap(currState->map_start, currState->fd_stat.st_size);
    } 
    free(currState);
    currState = NULL;
    exit(0);
}

typedef struct MENU {
  char *name;
  void (*fun)(state*);
}MENU;

int main(int argc, char** argv){

    struct MENU menu[] = { { "Toggle Debug Mode", toggle_debug}, 
                           { "Examine ELF File", examine_elf}, 
                           { "Quit", quit}, { NULL, NULL }  };
    
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    

    state* mystate = (state*) malloc(sizeof(state));
    mystate->debug_mode = 0;
    mystate->curr_fd = -1; 
    mystate->header = NULL; 
    mystate->map_start = NULL;
    
    while(1){
    print_deubg(mystate);
    printf("\nChoose action:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d-%s\n", i,menu[i].name);

    fgets(choice,MAX,stdin);
    index = atoi(choice) ;

    if( index < lowBound   ||   index > highBound ){
        printf("Not within bounds\n");
        exit(0);
    }

    menu[index].fun(mystate);

    printf("\n");

    }
    return 0;
}