#include "linux/limits.h"
#include "errno.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char** argv){
    char *buf;
    int pipeArr[2];
    // pipeArr[0] := read
    // pipeArr[1] := write
    int pid;
    char* msg = "hello\n";

    if(pipe(pipeArr) == -1 )
        exit(1);

    if( (pid = fork())== -1 )
        exit(1);

    if(pid == 0){ //child
        close(pipeArr[0]);
        write(pipeArr[1], msg, strlen(msg));
        close(pipeArr[1]);
        exit(0);
    }
    else{ //parent
        waitpid(-1,NULL,0);
        close(pipeArr[1]);
        while (read(pipeArr[0], &buf, 1) > 0)
            write(STDOUT_FILENO, &buf, 1);
    }
        return 0;
}