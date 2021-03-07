#include "linux/limits.h"
#include "LineParser.c"
#include "errno.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 2048
#define STDIN_FD 0
#define STDOUT_FD 1

int debugMode = 0;


int checkRegExe(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
        if(chdir(pCmdLine->arguments[1]))
            fprintf(stderr,"change directory failed\n");
        return 0;
    }
    return 1;
}

void execute(cmdLine *pCmdLine){
    int pid = fork();
    if(pid == -1) exit(1);
    if(!pid){ 
        if(pCmdLine->inputRedirect){
            close(STDIN_FD);
            fopen(pCmdLine->inputRedirect, "r+");
        }
        if(pCmdLine->outputRedirect){
            close(STDOUT_FD);
            fopen(pCmdLine->outputRedirect, "r+");
        }
        if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        _exit(1);  
    } 
    if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);    
    if(pCmdLine->blocking) waitpid(-1, NULL,0);          
                    
}

void quitCheck (char* buf){
    if(!strncmp("quit", buf, 4))    
        exit(0);
}

void printCWD (){
    char buf[MAX_SIZE];
    getcwd(buf, MAX_SIZE); 
    printf("The current working directory is: %s\n", buf);
}

void printExecutingCommand(cmdLine* pLine){
    fprintf(stderr,"Executing command: ");
    for(int i=0; i<pLine->argCount; i++)
        fprintf(stderr,"%s ", pLine->arguments[i]);
    fprintf(stderr,"\n");    
    fprintf(stderr, "input:  %s\n", pLine->inputRedirect );
    fprintf(stderr, "output: %s\n", pLine->outputRedirect );
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
        parsedLine = parseCmdLines(buf);
        if(debugMode) printExecutingCommand(parsedLine);
        if(checkRegExe(parsedLine))
            execute(parsedLine);
        freeCmdLines(parsedLine);
    }

    return 0;
} 

