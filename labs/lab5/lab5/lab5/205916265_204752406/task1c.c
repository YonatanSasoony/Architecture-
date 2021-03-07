#include "linux/limits.h"
#include "LineParser.c"
#include "errno.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 2048

int debugMode = 0;

void execute(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
        if(chdir(pCmdLine->arguments[1]))
            fprintf(stderr,"change directory failed\n");
    }
    else{
        int pid = fork();
        if(pid == -1) exit(1);
        if(!pid){ //isChild?
            if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            _exit(1);  
        } 
        if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);    
        if(pCmdLine->blocking) waitpid(-1, NULL,0);          
    }                       
}

void quitCheck (char* buf){
    if(!strncmp("quit", buf, 4))    
        exit(0);
}

void printCWD (){
    char buf[MAX_SIZE];
    getcwd(buf, MAX_SIZE); // return into buf the pathname, which is the current working directory of the calling process
    printf("The current working directory is: %s\n", buf);
}

void printExecutingCommand(cmdLine* pLine){
    fprintf(stderr,"Executing command: ");
    for(int i=0; i<pLine->argCount; i++)
        fprintf(stderr,"%s ", (char*)pLine->arguments[i]);
    fprintf(stderr,"\n");    
}

int main(int argc, char** argv){
    
    FILE* input = stdin;
    char buf[MAX_SIZE];
    cmdLine* parsedLine;

    for(int i=1; i<argc; i++)
        if(!strncmp(argv[i],"-d",2)) debugMode = 1;

    while(1){
        printCWD();
        printf("\nEnter next command\n");
        fgets(buf, MAX_SIZE, input);
        if(feof(input)) exit(0);
        quitCheck(buf);
        parsedLine = parseCmdLines(buf); //Returns a parsed structure cmdLine from a given strLine string
        if(debugMode) printExecutingCommand(parsedLine);
        execute(parsedLine);
        freeCmdLines(parsedLine);//Releases the memory that was allocated to accomodate the linked list of cmdLines
    }

    return 0;
} 

// exit() flushes io buffers and does some other things like run functions registered
// by atexit(). exit() invokes _end( )
// _exit() just ends the process without doing that. 
//You call _exit() from the parent process when creating a daemon for example.
