#include "util.h"
#define O_RDRW 2
#define Sys_open 5
#define Sys_lseek 19
#define SEEK_CUR 1
#define Sys_write 4
#define Sys_close 6
#define Sys_exit 1

void exit (){system_call(Sys_exit, 0x55,0,0); }

int main (int argc , char* argv[]){

    int file;

    if( (file = system_call(Sys_open,"greeting", O_RDRW, 0777)) < 0) exit();
      
    if(system_call(Sys_lseek, file, 0x291, SEEK_CUR) < 0) exit();

    if(system_call(Sys_write, file, argv[1], strlen(argv[1]) ) != 7) exit();

    if(system_call(Sys_close, file) < 0) exit();   

    return 0;
}
