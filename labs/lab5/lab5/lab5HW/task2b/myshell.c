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

void printList(process** process_list){
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

void freeProcessList(process* process_list){
    if(process_list != NULL){
        freeCmdLines(process_list->cmd);
        freeProcessList(process_list->next);
        free(process_list);
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    while(process_list != NULL){
        if( (process_list->pid = pid) ){
            process_list->status = status;
            break;
        }
        process_list = process_list->next;
    }
}

void updateProcessList(process **process_list){
    process* curPro = *process_list;
    while(curPro != NULL){
        int wstatus;  
        int retVal = waitpid(curPro->pid, &wstatus,WNOHANG);
        if(retVal == -1) perror("ERROR");
        else if (retVal > 0){ //changed
            if(WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) 
                updateProcessStatus(curPro, curPro->pid, TERMINATED);

            if(WIFSTOPPED(wstatus)) 
                updateProcessStatus(curPro, curPro->pid, SUSPENDED);

            if(WIFCONTINUED(wstatus)) 
                updateProcessStatus(curPro, curPro->pid, RUNNING);
        }
        curPro = curPro->next;
    }
}

void freePro(process* pro){
    freeCmdLines(pro->cmd);
    free(pro);
}
void printProcessListHelper(process** process_list, process* prev, int index){
    process* cur  = *process_list;
    if(cur == NULL) return;
    else if(prev == NULL && cur->status == TERMINATED){
        printPro(index, cur);
        *process_list = cur->next;
        freePro(cur);
        printProcessListHelper(process_list,prev,++index);
    }
    else if(prev == NULL && cur->status != TERMINATED){
        printPro(index, cur);
        prev = cur;
        cur = cur->next;
    }
    else if(cur->status == TERMINATED){
            printPro(index, cur);
            prev->next = cur->next;
            freePro(cur);
            printProcessListHelper(&cur->next, prev,++index);
        }
    else {
        printPro(index, cur);
        printProcessListHelper(&cur->next, prev->next, ++index);
    }
}

void printProcessList(process** process_list){
    updateProcessList(process_list);
    printf("INDEX\t\tPID\t\tSTATUS\t\tCOMMAND");
    printProcessListHelper(process_list, NULL,0);
}

int isCd(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )) 
        return 1;
    else 
        return 0;
}
int isProcs(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 1 && (!strncmp(pCmdLine->arguments[0],"procs",5)))
        return 1;
    else
        return 0;
}
void execute(cmdLine *pCmdLine){
        int pid = fork();
        if(pid == -1) exit(1);
         if(!isCd(pCmdLine) && !isProcs(pCmdLine))
            addProcess(&processList, pCmdLine, pid);
        //child code
        if(!pid){ 
            if(isCd(pCmdLine) || isProcs(pCmdLine)) exit(0);
            if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments) 
            _exit(1);  
        }
        //parent code
        if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);  
        int wstatus;  
        if(pCmdLine->blocking) waitpid(-1, &wstatus,0);
        if(isCd(pCmdLine)){
            if(chdir(pCmdLine->arguments[1]))
                fprintf(stderr,"change directory failed\n");
        }
        if(isProcs(pCmdLine)) printProcessList(&processList);
}

void quitCheck (char* buf){
    if(!strncmp("quit", buf, 4)){
        freeProcessList(processList);
        exit(0);
    }    
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
        if(feof(input)) {freeProcessList(processList); exit(0);};
        quitCheck(buf);
        parsedLine = parseCmdLines(buf);
        if(debugMode) printExecutingCommand(parsedLine,stderr); 
        execute(parsedLine);
    }
    
    return 0;
} 
