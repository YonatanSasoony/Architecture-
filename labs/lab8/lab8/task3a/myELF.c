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


#define BUFFER_SIZE 16
char buffer[BUFFER_SIZE];
char *itoa(int num)
{
	char* p = buffer+BUFFER_SIZE-1;
	int neg = num<0;
	if(neg)
	{
		num = -num;
	}
	*p='\0';
	do {
		*(--p) = '0' + num%10;
	} while(num/=10);
	if(neg) 
	{
		*(--p) = '-';
	}
	return p;
}

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

char* get_type_name(int type){
    if(type == SHT_NULL) return "NULL";
    else if(type == SHT_PROGBITS) return "PROGBITS";
    else if(type == SHT_SYMTAB) return "SYMTAB";
    else if(type == SHT_STRTAB ) return "STRTAB";
    else if(type == SHT_RELA) return "RELA";
    else if(type == SHT_HASH) return "HASH";
    else if(type == SHT_DYNAMIC) return "DYNAMIC";
    else if(type == SHT_NOTE) return "NOTE";
    else if(type == SHT_NOBITS) return "NOBITS";
    else if(type == SHT_REL) return "REL";
    else if(type == SHT_SHLIB) return "SHLIB";
    else if(type == SHT_DYNSYM) return "DYNSYM";
    else if(type == SHT_INIT_ARRAY) return "INIT_ARRAY";
    else if(type == SHT_FINI_ARRAY) return "FINI_ARRAY";
    else if(type == SHT_PREINIT_ARRAY) return "PREINIT_ARRAY";
    else if(type == SHT_GROUP) return "GROUP";
    else if(type == SHT_SYMTAB_SHNDX) return "SYMTAB_SHNDX";
    else if(type == SHT_NUM) return "NUM";
    else if(type == SHT_LOOS) return "LOOS";
    else return "unknown";
}

char* get_name(int offset, state* currState){
    int sh_idx_name = currState->header->e_shstrndx;
    Elf32_Shdr* sh_names = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + sh_idx_name * currState->header->e_shentsize);
    //sh_names->sh_offset := offset of string table from the head of the file
    return (char*) (currState->map_start + sh_names->sh_offset + offset);
}

void print_section_names (state* currState){
    if(currState->curr_fd == -1){
        printf("current fd is invalid\n");
        return;
    }
    printf("index\t name\t\t\taddr\t\toffset\tsize\ttype\n");
    for(int i=0; i<currState->header->e_shnum; i++){
        Elf32_Shdr* curr = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + i * currState->header->e_shentsize); 
        printf("[%02d]\t%-*s\t\t%08X\t%06X\t%06X\t%s\n", i,10,get_name(curr->sh_name,currState), curr->sh_addr,curr->sh_offset,curr->sh_size, get_type_name(curr->sh_type));
    }
    if(currState->debug_mode){
        printf("\nshstrndx: %d\n",currState->header->e_shstrndx);
        int sh_idx_name = currState->header->e_shstrndx;
        Elf32_Shdr* sh_names = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + sh_idx_name * currState->header->e_shentsize);
        printf("section name offsets: %X\n",sh_names->sh_offset);
    }
}

int get_symbol_table_index(state* currState){
    for(int i=0; i<currState->header->e_shnum;i++){
        Elf32_Shdr* curr = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + i * currState->header->e_shentsize);
        if(curr->sh_type == SHT_SYMTAB)
            return i;
    }
    return -1 ;
}

int get_str_table_index(state* currState){
    for(int i=0; i<currState->header->e_shnum;i++){
        Elf32_Shdr* curr = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + i * currState->header->e_shentsize);
        if(!strncmp(get_name(curr->sh_name, currState),".strtab",7))
            return i;
    }
    return -1 ;
}

char* get_section_name (int index, state* currState){
    if(index == SHN_UNDEF) return "UNDEF";
    if(index == SHN_LORESERVE) return "LORESERVE";
    if(index == SHN_LOPROC) return "LOPROC";
    if(index == SHN_HIPROC) return "HIPROC";
    if(index == SHN_ABS) return "ABS";
    if(index == SHN_COMMON) return "COMMON";
    if(index == SHN_HIRESERVE) return "HIRESERVE";
    int sh_idx_name = currState->header->e_shstrndx;
    Elf32_Shdr* sh_names = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + sh_idx_name * currState->header->e_shentsize);
    Elf32_Shdr* sh_entry = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + index * currState->header->e_shentsize);
    char* name = (char*) (currState->map_start + sh_names->sh_offset + sh_entry->sh_name);
    return name;
}

char* get_symbol_name(int table_offset,int name_offset ,state* currState){
    return (char*)(currState->map_start + table_offset + name_offset);
}
  
char* get_index(int index){
    if(index == SHN_UNDEF) return "UND";
    if(index == SHN_LORESERVE) return "LORESERVE";
    if(index == SHN_LOPROC) return "LOPROC";
    if(index == SHN_HIPROC) return "HIPROC";
    if(index == SHN_ABS) return "ABS";
    if(index == SHN_COMMON) return "COMMON";
    if(index == SHN_HIRESERVE) return "HIRESERVE";
    return itoa(index);
}
int get_symbol_section_index (state* currState, int starting_index){
    for(int i=starting_index; i<currState->header->e_shnum;i++){
        Elf32_Shdr* curr = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + i * currState->header->e_shentsize);
        if((curr->sh_type == SHT_SYMTAB) || (curr->sh_type == SHT_DYNSYM))
            return i;
    }
    return -1;
}

void print_symbols (state* currState){
    if(currState->curr_fd == -1){
        printf("current fd is invalid\n");
        return;
    }
    int starting_index = 0;
    int symbol_section_index;
    if(get_symbol_section_index(currState,0) == -1){
        printf("NO symbol tables\n");
        return;
    }
    while((symbol_section_index = get_symbol_section_index(currState,starting_index)) != -1){
        starting_index = symbol_section_index + 1;
        
        Elf32_Shdr* symbol_table_entry = (Elf32_Shdr*)(currState->map_start + currState->header->e_shoff + symbol_section_index * currState->header->e_shentsize);
        int symbol_table_offset = symbol_table_entry->sh_offset;

        int str_table_index = symbol_table_entry->sh_link;
        int str_table_offset = ((Elf32_Shdr*) ( currState->map_start + currState->header->e_shoff + str_table_index * currState->header->e_shentsize ))->sh_offset; 
        
        int size = 0;
        int index = 0;
        if(currState->debug_mode) printf("\ntable size: %d\tnumber of symbols: %ld\n", symbol_table_entry->sh_size,symbol_table_entry->sh_size/sizeof(Elf32_Sym));
        printf("\nindex\tvalue\t\tsec_index\tsection name\t\tsymbol name\n");
        while(size < symbol_table_entry->sh_size){
            Elf32_Sym* curr = (Elf32_Sym*)(currState->map_start + symbol_table_offset + size);
            size += sizeof(Elf32_Sym);
            printf("[%02d]\t%08X\t%s\t\t%s\t\t\t%s\n", index++, curr->st_value, get_index(curr->st_shndx), get_section_name(curr->st_shndx, currState), get_symbol_name(str_table_offset,curr->st_name ,currState));
        }
    }
}
int get_relocation_section_index (state* currState, int starting_index){
    for(int i=starting_index; i<currState->header->e_shnum;i++){
        Elf32_Shdr* curr = (Elf32_Shdr*) (currState->map_start + currState->header->e_shoff + i * currState->header->e_shentsize);
        if(curr->sh_type == SHT_REL)
            return i;
    }
    return -1;
}

void relocation_tables (state* currState){
    printf("Offset\t\tInfo\n");
    int starting_index = 0;
    int relocation_section_index;
    while((relocation_section_index = get_relocation_section_index(currState,starting_index)) != -1){
        starting_index = relocation_section_index + 1;
        Elf32_Shdr* relocation_entry = (Elf32_Shdr*)(currState->map_start + currState->header->e_shoff + relocation_section_index * currState->header->e_shentsize);
        int offset_relocation_table = relocation_entry->sh_offset;
        int size = 0;
        while(size < relocation_entry->sh_size){
            Elf32_Rel* curr = (Elf32_Rel*)(currState->map_start + offset_relocation_table + size);
            size += sizeof(Elf32_Rel);
            printf("%08X\t%08X\n",curr->r_offset , curr->r_info);
        }
        printf("************************\n");
    }
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
                           {"Print Section Names", print_section_names},
                           {"Print Symbols", print_symbols},
                           {"Relocation Tables", relocation_tables},
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