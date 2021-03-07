#include "linux/limits.h"
#include "LineParser.c"
#include "errno.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
#define MAX_SIZE 2048

typedef struct process{
    cmdLine* cmd;        
    pid_t pid; 		       
    int status;            
    struct process *next;  
} process;

int debugMode = 0;
process* processList = NULL ;

void printExecutingCommand(cmdLine* pLine,FILE* output){
    for(int i=0; i<pLine->argCount; i++)
        fprintf(output, "%s ", (char*)pLine->arguments[i]);
    if(output == stderr) fprintf(output,"\n");  
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* newPro = (process*) malloc(sizeof(process));
    newPro->cmd = cmd;
    newPro->pid = pid;
    newPro->status = RUNNING;
    newPro->next = NULL;
    if(*process_list == NULL) *process_list = newPro;
    else {
        process* curPro = *process_list;
        while ( curPro->next != NULL)
            curPro = curPro->next ;
        curPro->next = newPro;
    }
}

char* convertStatus (int status){
    if(status == TERMINATED) return "TERMINATED";
    else if(status == RUNNING) return "RUNNING";
    else if(status == SUSPENDED) return "SUSPENDED";
    else return "";
}

void printPro(int index, process* pro){
    printf("\n%d\t\t%d\t\t%s\t\t", index, pro->pid, convertStatus(pro->status));
    printExecutingCommand(pro->cmd, stdout);
}

void printProcessList(process** process_list){
    printf("INDEX\t\tPID\t\tSTATUS\t\tCOMMAND");
    if(process_list != NULL){
        process* curPro = *process_list;
        int index = 0;
        while(curPro != NULL){
            printPro(index++, curPro);
            curPro = curPro->next;
        }
    }
}

void executeNextCommand(cmdLine *pCmdLine){
        int pid = fork();
        if(pid == -1) exit(1);
        if(!pid){ //child code
            if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            _exit(1);  
        }//parent code
        if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);  
        int wstatus;  
        if(pCmdLine->blocking) waitpid(-1, &wstatus,0);
}

void execute(cmdLine *pCmdLine){
    
    addProcess(&processList, pCmdLine, (pid_t)0);

    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
        if(chdir(pCmdLine->arguments[1]))
            fprintf(stderr,"change directory failed\n");
    }
    else if(pCmdLine->argCount == 1 && (!strncmp(pCmdLine->arguments[0],"procs",5) )){
            printProcessList(&processList);
    }
    else         
        executeNextCommand(pCmdLine);                    
}

void quitCheck (char* buf){
    if(!strncmp("quit", buf, 4))    
        exit(0);
}

void printCWD (){
    char buf[MAX_SIZE];
    getcwd(buf, MAX_SIZE); 
    printf("\nThe current working directory is: %s\n", buf);
}

void getLine(char* buf, FILE* input){
    printf("Enter next command\n");
    fgets(buf, MAX_SIZE, input);
}

int main(int argc, char** argv){
    
    FILE* input = stdin;
    char buf[MAX_SIZE];
    cmdLine* parsedLine = NULL;
    
    for(int i=1; i<argc; i++)
        if(!strncmp(argv[i],"-d",2)) debugMode = 1;

    while(1){
        printCWD();
        getLine(buf,input); 
        if(feof(input)) exit(0);
        quitCheck(buf);
        parsedLine = parseCmdLines(buf);
        if(debugMode) printExecutingCommand(parsedLine,stderr); 
        execute(parsedLine);
    }
    freeCmdLines(parsedLine);
    return 0;
} 
