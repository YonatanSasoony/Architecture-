#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <arpa/inet.h>

#define MAX 1024
#define MAX_FILE_NAME_SIZE 100
#define New_Line printf("\n");

typedef struct {
  char debug_mode; 
  char file_name[128];
  int unit_size; //Size can be either 1, 2 or 4, with 1 as the default. 
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
  int file_size;
} state;

char* display_format(unsigned char mode) {
    char* formats[] = {"%d", "%X"};
    return formats[mode];
}  

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
    if(currState->debug_mode){
        char* format = display_format(currState->display_mode);
        printf("unit size:");
        printf(format, currState->unit_size);
        printf("\nfile name:%s\nmem count:", currState->file_name);
        printf(format, (int)currState->mem_count);
        New_Line
    }
} 

void set_file_name (state* currState){
    printf("Please enter a file name:\n");
    fgets(currState->file_name, MAX_FILE_NAME_SIZE, stdin);
    sscanf(currState->file_name, "%s", currState->file_name);
    if(currState->debug_mode)
        printf("Debug: file name set to '%s'\n", currState->file_name);
}

int is_valid_unit(int unit){
    if( (unit == 1) || (unit == 2) || (unit == 4) )
        return 1;
    else 
        return 0;
} 

void set_unit_size (state* currState){
    int unit = 0;
    char user_choice[MAX];

    printf("Please enter a number:\n");
    fgets(user_choice, MAX, stdin);
    sscanf(user_choice,"%d",&unit);

    if(is_valid_unit(unit)){
        currState->unit_size = unit;
        if(currState->debug_mode){
            printf("Debug: set size to ");
            printf(display_format(currState->display_mode), currState->unit_size);
            New_Line
        }
    }
    else
        printf("invalid unit size\n");
}

void quit (state* currState){
    if(currState->debug_mode)
        printf("quitting\n");
    free(currState);
    currState = NULL;
    exit(0);
}

void load_into_memory (state* currState){
    if(!strcmp(currState->file_name,"")){
        printf("ERROR: file name is empty\n");
        return;
    }
    FILE* file = fopen(currState->file_name, "r");
    if(file == NULL){
        printf("ERROR: could not open the file\n");
        return;
    }
    int loc;
    int len;
    char buf[MAX];
    printf("Please enter <location> <length>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%X %d",(unsigned int *)&loc,&len);
    char* format = display_format(currState->display_mode);
    if(currState->debug_mode){
        printf("file name:%s\nlocation:", currState->file_name);
        printf(format, loc);New_Line
        printf("length:");
        printf(format, len);New_Line
    }    
    fseek(file,loc,SEEK_SET); 
    fread(currState->mem_buf, currState->unit_size, len, file);
    printf("Loaded ");
    printf(format, len*currState->unit_size); 
    printf(" units into memory\n");
    printf("***** memory location of mem_buf: %p *****",currState->mem_buf);

    fseek(file,0,SEEK_SET); 
    fseek(file, 0, SEEK_END);
    currState->file_size = ftell(file);
    currState->mem_count = len*currState->unit_size;
    fclose(file);
}

void toggle_display(state* currState){
    if(currState->display_mode){
        currState->display_mode = 0;
        printf("Display flag now off, decimal representation");
    }
    else{
        currState->display_mode = 1;
        printf("Display flag now on, hexadecimal representation");        
    }
} 

/* Prints the buffer to screen by converting it to text with printf */
void print_units(unsigned char* buffer, int count, int unit_size, char* format) {
    unsigned char* end = buffer + unit_size*count;
    while (buffer < end) {
        if (unit_size == 1){
            unsigned char var1 = *((unsigned char*)(buffer));
            printf(format, var1);
            New_Line
            buffer += unit_size;
        }
        if (unit_size == 2){
            unsigned short var2 = *((unsigned short*)(buffer));
            printf(format, var2);
            New_Line
            buffer += unit_size;
        }
        if (unit_size == 4){
            unsigned int var4 = *((unsigned int*)(buffer));
            printf(format, var4);
            New_Line
            buffer += unit_size;
        } 
    }
}

void print_display_type(char mode){
    if (mode)
        printf("Hexadecimal\n");
    else
        printf("Decimal\n");
    printf("======\n"); 
}

void memory_display(state* currState){
    int u;
    unsigned char* addr;
    char buf[MAX];
    char* format = display_format(currState->display_mode);
    printf("Please enter <number of units> <addr>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%d %p",&u,&addr);
    print_display_type(currState->display_mode);
    if(addr == 0)
        addr = currState->mem_buf;

    print_units(addr,u,currState->unit_size,format);
}  
 
void save_into_file(state* currState){
    int offset;
    int len;
    unsigned char* addr;
    char buf[MAX];
    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%p %X %d", &addr, &offset, &len);
   
    if(addr == 0)
        addr = currState->mem_buf;
    

    FILE* file = fopen(currState->file_name, "r+");
    if(file == NULL){
         if(currState->debug_mode)
            printf("ERROR: could not open the file\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    if(offset > ftell(file)){
        printf("ERROR: offset is greater than size\n");
        return;
    }
     
    fseek(file,offset,SEEK_SET);
    fwrite(addr, currState->unit_size, len, file);       

    fclose(file);
    
}

int power(int x, int y){
    int result = 1;
    for (int i=0; i<y ;i++)
        result *= x;
    return result;
}

void memory_modify(state* currState){
    int loc;//offset
    int val;
    char buf[MAX];
    printf("Please enter <location> <value>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%X %X", &loc, &val);
    if(currState->debug_mode){
        char* format = display_format(currState->display_mode);
        printf("location:");
        printf(format, loc);
        printf("\nvalue:");
        printf(format, val);
        New_Line
    }
    if( (val < 0) || (val >= power(256,currState->unit_size) ) ){
        printf("ERROR: val size is not equal to unit size");
        return;
    }
    if( (int)(currState->mem_count - loc) < currState->unit_size){
        printf("ERROR: not enough space for writing to mem.buf");
        return;
    }
    memmove(currState->mem_buf+loc, &val, currState->unit_size);
}

typedef struct MENU {
  char *name;
  void (*fun)(state*);
}MENU;

int main(int argc, char** argv){

    struct MENU menu[] = { {"Toggle Debug Mode", toggle_debug}, 
                           {"Set File Name", set_file_name}, 
                           {"Set Unit Size", set_unit_size},
                           {"Load Into Memory", load_into_memory},
                           {"Toggle Display Mode", toggle_display},
                           {"Memory Display", memory_display},
                           {"Save Into File", save_into_file},
                           {"Memory Modify", memory_modify},
                           {"Quit", quit}, 
                           {NULL, NULL }  };
    
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    

    state* mystate = (state*) malloc(sizeof(state));
    mystate->debug_mode = 0;
    mystate->unit_size = 1;
    mystate->display_mode =0;
    //init more fields

    while(1){
    print_deubg(mystate);
    printf("\nChoose action:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d-%s\n", i,menu[i].name);

    fgets(choice,MAX,stdin);
    sscanf(choice,"%d",&index);
    

    if( index < lowBound   ||   index > highBound ){
        printf("Not within bounds\n");
        exit(0);
    }

    menu[index].fun(mystate);

    printf("\n");

    }
    return 0;
}