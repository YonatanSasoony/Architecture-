#include "util.h"

#define Sys_exit 1
#define Sys_read 3
#define Sys_write 4
#define Sys_open 5
#define Sys_close 6
#define Sys_lseek 19

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define O_WRONLY 1
#define O_CREAT 64
#define O_RDONLY 0
#define O_RDRW 2

#define SEEK_CUR 1
#define newLine 10
#define a_ascii 97
#define z_ascii 122
#define MAX_LENGTH 1024

extern int system_call();

int isD = 0;

void safe2 (int sysRetVal, char* errText){
    if(sysRetVal<0){
        system_call(Sys_write,STDOUT,errText,strlen(errText));
        system_call(Sys_write,STDOUT,"\n",1);
        system_call(Sys_exit, 0x55,0 ,0);
    }
}

void debugPrint(int sysRetVal, int id){
    system_call(Sys_write,STDERR,"system call id: ",16);
    char* idStr = itoa(id);
    system_call(Sys_write,STDERR,idStr,strlen(idStr));
    system_call(Sys_write,STDERR,"\tretrun code value: ",20);
    char* sysRetValStr = itoa(sysRetVal);
    system_call(Sys_write,STDERR,sysRetValStr,strlen(sysRetValStr));
    system_call(Sys_write,STDERR,"\n",1);
}

int id (char* errText){
    if(!strcmp(errText, "writeErr")) return 4;
    else if(!strcmp( errText, "openOutputFileErr")) return 5;
    else if(!strcmp( errText, "openInputFileErr")) return 5;
    else if(!strcmp( errText, "readErr")) return 3;
    else if(!strcmp( errText, "closeErr")) return 6;
    else return -1;    
}

int safe (int sysRetVal, char* errText){
    if(sysRetVal<0){
        system_call(Sys_write,STDOUT,errText,strlen(errText));
        system_call(Sys_write,STDOUT,"\n",1);
        system_call(Sys_exit, 0x55,0 ,0);
    }

    if(isD) debugPrint(sysRetVal, id(errText));

    return sysRetVal;
}

void debugPrintInputOutput(int isO, int isI, char* outputFile, char* inputFile){

        safe2(system_call(Sys_write, STDERR, "The output file is: ", 20), "writeErr");
        if(!isO) safe2(system_call(Sys_write, STDERR, "stdout", 6), "writeErr");
        else     safe2(system_call(Sys_write, STDERR, outputFile, strlen(outputFile)), "writeErr");
        safe2(system_call(Sys_write, STDERR, "\n", 1), "writeErr");

        safe2(system_call(Sys_write, STDERR, "The input  file is: ", 20), "writeErr");
        if(!isI) safe2(system_call(Sys_write, STDERR, "stdin", 5), "writeErr");
        else     safe2(system_call(Sys_write, STDERR, inputFile, strlen(inputFile)), "writeErr");
        safe2(system_call(Sys_write, STDERR, "\n", 1), "writeErr");
    
}

int upperCase (char c){
    return c >= a_ascii && c <= z_ascii;
}

void cleanstr(char str[]){
    int i;
    for(i=0; i<MAX_LENGTH;i++)
        str[i] = '\0';
}

int isLineEmpty (char str[]){
    int i;
    for (i=0; i<strlen(str); i++)
        if(str[i] != '\0')
            return 0;
    return 1;
}

void printLine (int isD, char str[], int output){
    if (!isLineEmpty(str)){
        safe(system_call(Sys_write,output,str, strlen(str)), "writeErr");
        cleanstr(str);
    }
}

void printLast(int isD, char str[], int output, int j){
     if (!isLineEmpty(str)){
         str[j] = '\n';
        safe(system_call(Sys_write,output,str, strlen(str)), "writeErr");
    }
}

int main (int argc , char* argv[]){

    char c[1];
    c[0] = 0;
    int i;
    char str[MAX_LENGTH]; cleanstr(str);
    int j = 0;
    int isO = 0;
    int isI = 0;
    int output = STDOUT;
    int input = STDIN;
    int argNumInput = 0;
    int argNumOutput = 0 ;

    for(i=1; i<argc; i++)
        if( (strncmp("-D", argv[i],2)==0) )
            isD = 1 ;

    for(i=1; i<argc; i++){
        if((strncmp("-o", argv[i],2)==0)){
            isO = 1;
            argNumOutput = i;
            output = safe(system_call(Sys_open, argv[i]+2, O_WRONLY | O_CREAT, 0777), "openOutputFileErr" );
        }
        else if((strncmp("-i", argv[i],2)==0)){
            isI = 1 ;
            argNumInput = i;
            input = safe(system_call(Sys_open, argv[i]+2, O_RDONLY, 0777), "openInputFileErr") ;
            }
        }    
    
    if(isD) debugPrintInputOutput(isO,isI, argv[argNumOutput]+2 , argv[argNumInput]+2);

    while( ( safe(system_call(Sys_read,input,c,1), "readErr") ) > 0){
    
        if(upperCase(c[0])) c[0] = c[0] - 32 ;
        str[j++] = c[0];

        if(c[0] == '\n'){ printLine(isD, str, output); j=0; }

    }printLast(isD, str, output, j);

    if(isO == 1)
       safe(system_call(Sys_close, output), "closeErr");   
    if(isI == 1)
        safe(system_call(Sys_close, input), "closeErr");   

    return 0;
}
