#include "linux/limits.h"
#include "LineParser.c"
#include "errno.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 2048
#define STDIN_FD 0
#define STDOUT_FD 1

typedef struct pair pair;
struct pair{
    char* name;
    char* value;
    pair *next;
};

int debugMode = 0;
pair* pairList = NULL;


void printExecutingCommand(cmdLine* pLine){
    fprintf(stderr,"Executing command: ");
    for(int i=0; i<pLine->argCount; i++)
        fprintf(stderr,"%s ", pLine->arguments[i]);
    fprintf(stderr,"\n");    
    fprintf(stderr, "input:  %s\n", pLine->inputRedirect );
    fprintf(stderr, "output: %s\n", pLine->outputRedirect );
}

void insertPair(char* name, char* value){
    pair* curr = pairList;
    pair* prev = NULL;

    if(curr == NULL){
        pair* newPair = (pair*) malloc (sizeof(pair));
        newPair->name = strClone(name);
        newPair->value = strClone(value);
        newPair->next = NULL;
        pairList = newPair;
    }
    else{
        while(curr != NULL){
            if( !strcmp(curr->name, name) ){
                free(curr->value);
                curr->value =  strClone(value);
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        if(curr == NULL){
            pair* newPair = (pair*) malloc (sizeof(pair));
            newPair->name = strClone(name);
            newPair->value = strClone(value);
            newPair->next = NULL;
            prev->next = newPair;
        }
    }
}

char* getValue(char* name){
    pair* curr = pairList;
    while(curr != NULL){
        if(!strcmp(curr->name,name))
            return curr->value;
        curr = curr->next;
    }
    return NULL;
}

void printVars(pair* pairList){
    if(pairList != NULL){
        printf("%s\t%s\n", pairList->name, pairList->value);
        printVars(pairList->next);
    }
} 

void freeList(pair* pairList){
    if(pairList != NULL){
        freeList(pairList->next);
        free(pairList->name);
        free(pairList->value);
        free(pairList);
        pairList->name = NULL;
        pairList->value = NULL;
        pairList->next = NULL;
    }
}

void replaceArguments(cmdLine *pCmdLine){
    char* var = NULL;
    char* val = NULL;
    for(int i=0; i<pCmdLine->argCount; i++){
        if( !strncmp(pCmdLine->arguments[i],"$",1) ){
            var = pCmdLine->arguments[i] + 1 ;
            val = getValue(var);
            if (val == NULL){
                fprintf(stderr, "%s NOT FOUND\n", var);
                //exit(1);
            }
            else{
                replaceCmdArg(pCmdLine, i, val);
                if(debugMode) printExecutingCommand(pCmdLine);
            }
        }
    }
}

int checkRegExe(cmdLine *pCmdLine){
    if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
        if(!strcmp(pCmdLine->arguments[1], "~")){
            if(debugMode) printf("HOME : %s\n", getenv("HOME"));
            if(chdir(getenv("HOME")))
                fprintf(stderr,"change directory failed\n");
        }
        else if(chdir(pCmdLine->arguments[1]))
                fprintf(stderr,"change directory failed\n");
        return 0;
    }
    if(pCmdLine->argCount >= 3 && (!strncmp(pCmdLine->arguments[0], "set", 3)) ){
        insertPair(pCmdLine->arguments[1], pCmdLine->arguments[2]);
        return 0;
    }
    if(pCmdLine->argCount == 1 && (!strncmp(pCmdLine->arguments[0], "vars", 4)) ){
        printVars(pairList);
        printf("\n");
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
    if(!strncmp("quit", buf, 4)){
        freeList(pairList);
        pairList = NULL;
        exit(0);
    }    
}

void printCWD (){
    char buf[MAX_SIZE];
    getcwd(buf, MAX_SIZE); 
    printf("The current working directory is: %s\n", buf);
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
        quitCheck(buf);
        parsedLine = parseCmdLines(buf);
        replaceArguments(parsedLine);
        if(debugMode) printExecutingCommand(parsedLine);
        if(checkRegExe(parsedLine))
            execute(parsedLine);
        freeCmdLines(parsedLine);
    }   

    return 0;
} 

