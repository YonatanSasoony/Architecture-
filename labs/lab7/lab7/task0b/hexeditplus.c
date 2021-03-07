#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024
#define MAX_FILE_NAME_SIZE 100

typedef struct {
  char debug_mode; 
  char file_name[128];
  int unit_size; //Size can be either 1, 2 or 4, with 1 as the default. 
  unsigned char mem_buf[10000];
  size_t mem_count;
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
        printf("unit size: %d\nfile name:%s\nmem count:%d\n",currState->unit_size,currState->file_name,(int)currState->mem_count);
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
        if(currState->debug_mode)
            printf("Debug: set size to %d\n", currState->unit_size);
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

typedef struct MENU {
  char *name;
  void (*fun)(state*);
}MENU;

int main(int argc, char** argv){

    struct MENU menu[] = { { "Toggle Debug Mode", toggle_debug}, { "Set File Name", set_file_name}, 
                           { "Set Unit Size", set_unit_size},{ "Quit", quit}, { NULL, NULL }  };
    
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    

    state* mystate = (state*) malloc(sizeof(state));
    mystate->debug_mode = 0;
    mystate->unit_size = 1;
    //init more fields
    
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