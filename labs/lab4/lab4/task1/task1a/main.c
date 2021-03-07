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

#define O_RDRW 2
#define SEEK_CUR 1
#define newLine 10
#define a_ascii 97
#define z_ascii 122
#define MAX_LENGTH 1024

extern int system_call();

int safe (int sysRetVal, char* errText){
    if(sysRetVal<0){
        system_call(Sys_write,STDOUT,errText,strlen(errText));
        system_call(Sys_write,STDOUT,"\n",1);
        system_call(Sys_exit, 0x55,0 ,0);
    }

    return sysRetVal;
}

void debugPrint(char* sysRetVal, char* id){
    safe(system_call(Sys_write,STDERR,"retrun code Value: ",19), "writeErr");
    safe(system_call(Sys_write,STDERR,sysRetVal,strlen(sysRetVal)), "writeErr");
    safe(system_call(Sys_write,STDERR,"\tsystem call id:",16), "writeErr");
    safe(system_call(Sys_write,STDERR,id,strlen(id)), "writeErr");
    safe(system_call(Sys_write,STDERR,"\n",1), "writeErr");
}

int upperCase (char c){
    return c >= a_ascii && c <= z_ascii;
}

void cleanstr(char str[]){
    int i;
    for(i=0; i<MAX_LENGTH;i++)
        str[i] = '\0';
}

int main (int argc , char* argv[]){

    char c[1];
    c[0] = 0;
    int sysSRetValRead;
    int sysSRetValWrite;
    int isD = 0;
    int i;
    char str[MAX_LENGTH];
    cleanstr(str);
    int j = 0;

    for(i=1; i<argc; i++){
        if( (strncmp("-D", argv[i],2)==0) )
            isD = 1 ;
    }
    
    while(( sysSRetValRead = safe (system_call(Sys_read,STDIN,c,1), "readErr") ) > 0){

        if(isD) debugPrint(itoa(sysSRetValRead), "3" );

        if(upperCase(c[0])) c[0] = c[0] - 32 ;
        str[j++] = c[0];

        if(c[0] == '\n'){
            sysSRetValWrite = safe(system_call(Sys_write,STDOUT,str, strlen(str)), "writeErr");
            if(isD) debugPrint(itoa(sysSRetValWrite), "4");
            j = 0;
            cleanstr(str);
        }
    }

    return 0;
}


