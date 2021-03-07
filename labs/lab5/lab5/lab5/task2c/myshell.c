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
        freeProcessList(process_list->next);
        freeCmdLines(process_list->cmd);
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
        int retVal = waitpid(curPro->pid, &wstatus, WNOHANG | WUNTRACED | WCONTINUED );

        if(retVal == -1)
            updateProcessStatus(curPro, curPro->pid, TERMINATED); 

        if (retVal > 0){ //changed
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

void printProcessListHelper(process** process_list,process* cur, process* prev, int index){


    if(cur == NULL) return;
    printPro(index, cur);
    if (cur->status == TERMINATED){
        if (prev == NULL){
            *process_list = cur->next;
            freePro(cur);
            cur = *process_list;
        }
        else{
            prev->next = cur->next;
            freePro(cur);
            cur = prev->next;
        }
        printProcessListHelper(process_list,cur,prev,++index);
    }
    else
        printProcessListHelper(process_list,cur->next,cur,++index);
}


void printProcessList(process** process_list){
    updateProcessList(process_list);
    printf("INDEX\t\tPID\t\tSTATUS\t\tCOMMAND");
    if (process_list != NULL)
        printProcessListHelper(process_list, *process_list, NULL,0);
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

int isSuspend(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"suspend",7)))
        return 1;
    else
        return 0;

}
int isKill(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"kill",4)))
        return 1;
    else
        return 0;

}
int isWake(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"wake",4)))
        return 1;
    else
        return 0;

}

int additionalCommand (cmdLine *pCmdLine){
    return (isCd(pCmdLine) || isProcs(pCmdLine)|| isSuspend(pCmdLine) || isKill(pCmdLine) || isWake(pCmdLine));
}

void sendSignal(int pid){
    int ret = kill(pid,SIGHUP);
    printf("ret: %d\n", ret);
}

void exeNewCommand(cmdLine *pCmdLine){

    if(isCd(pCmdLine)){
        if(chdir(pCmdLine->arguments[1]))
            fprintf(stderr,"change directory failed\n");
    }

    if(isProcs(pCmdLine)) 
        printProcessList(&processList);

    if(isKill(pCmdLine)){
        int ret = kill(atoi(pCmdLine->arguments[1]),SIGINT);
        printf("ret: %d\n", ret);
        if(ret == -1) perror("ERROR:");

    }
    
    if(isSuspend(pCmdLine)){
        int ret = kill(atoi(pCmdLine->arguments[1]),SIGTSTP);
        printf("ret: %d\n", ret);
        if(ret == -1) perror("ERROR:");
    }

    if(isWake(pCmdLine)){
        int ret = kill(atoi(pCmdLine->arguments[1]),SIGCONT);
        printf("ret: %d\n", ret);
        if(ret == -1) perror("ERROR:");
    }
}

void execute(cmdLine *pCmdLine){
       
        int pid = fork();
        if(pid == -1) exit(1);
        
        //child code
        if(!pid){ 
            if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            _exit(1);  
        }
        //parent code
        if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);

        if(pCmdLine->blocking) waitpid(-1, NULL,0);

        addProcess(&processList, pCmdLine, pid);   
        
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
        if(additionalCommand(parsedLine)){
            exeNewCommand(parsedLine);
            freeCmdLines(parsedLine);
        }
        else
            execute(parsedLine);  
    }

    return 0;
} 
