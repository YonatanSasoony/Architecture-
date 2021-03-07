#include "linux/limits.h"
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


int main (int argc, char** argv){
    int pipeArr[2];
    int pid1;
    int pid2;
    int newFD1;
    int newFD2;
    char* child1Args[] = {"ls", "-l", NULL};
    char* child2Args[] = {"tail" , "-n", "2", NULL};

    int debugMode = 0;
    for(int i=1; i<argc; i++)
        if(!strncmp(argv[i],"-d",2)) debugMode = 1;


    if(pipe(pipeArr) == -1 ){
         if(debugMode) fprintf(stderr,"(parent_process>exiting…)\n");
         exit(1);
    }
        

    if(debugMode) fprintf(stderr, "(parent_process>forking…)\n");
    if( (pid1 = fork())== -1 ){
         if(debugMode) fprintf(stderr,"(parent_process>exiting…)\n");
         exit(1);
    }
    if(debugMode) fprintf(stderr, "(parent_process>created process with id:%d)\n", pid1);

    if(pid1 == 0){ //child1
       if(debugMode) fprintf(stderr,"(child1>redirecting stdout to the write end of the pipe…)\n" ); 
       close(STDOUT_FD);
       newFD1 = dup(pipeArr[1]);
       if(debugMode) fprintf(stderr,"(child1>going to execute cmd:ls -l)\n") ; 
       execvp(child1Args[0],child1Args);
       close(newFD1);
    }
    else{ //parent
        if(debugMode) fprintf(stderr,"(parent_process>waiting for child processes to terminate…)\n" ); 
        waitpid(-1,NULL,0);//wait for child1
        if(debugMode) fprintf(stderr,"(parent_process>closing the write end of the pipe…)\n"); 
        close(pipeArr[1]);
        if( (pid2 = fork())== -1 ){
         if(debugMode) fprintf(stderr,"(parent_process>exiting…)\n");
         exit(1);
        }      
        if(pid2 == 0){//child2
        if(debugMode) fprintf(stderr,"(child2>redirecting stdin to the read end of the pipe…)\n"  ); 
            close(STDIN_FD);
            newFD2 = dup(pipeArr[0]);
            if(debugMode) fprintf(stderr,"(child2>going to execute cmd:tail -n 2)\n") ;
            execvp(child2Args[0],child2Args);
            close(newFD2);
        }
        else{
            if(debugMode) fprintf(stderr,"(parent_process>closing the read end of the pipe…)\n" ); 
            close(pipeArr[0]);
            if(debugMode) fprintf(stderr,"(parent_process>waiting for child processes to terminate…)\n" );
            waitpid(-1,NULL,0);//wait for child2
        }
    }
        if(debugMode) fprintf(stderr,"(parent_process>exiting…)\n");
        return 0;
}