#include "linux/limits.h"
#include "LineParser.c"
#include "errno.h"
#include "unistd.h"

#define MAX_SIZE 2048

void execute(cmdLine *pCmdLine){
    if( execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1 ) // The  first  argument, should  point  to the filename associated with the file being executed.
        perror("Error");                                            // not like execv, execvp know to search in the right env
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

void printCmdLine(cmdLine* Line){
    printf("%s\n", (char*)Line->arguments);
    printf("%d\n", Line->argCount);
    //printf("%s\n", Line->inputRedirect);
    //printf("%s\n", Line->outputRedirect);
    printf("%c\n", Line->blocking);
    printf("%d\n", Line->idx);
    //if(Line->next != NULL) printCmdLine(Line->next);
}

int main(int argc, char** argv){
    
    FILE* input = stdin;
    char buf[MAX_SIZE];
    cmdLine* parsedLine;

    while(1){
        printCWD();
        printf("Enter next command\n");
        fgets(buf, MAX_SIZE, input);
        quitCheck(buf);
        parsedLine = parseCmdLines(buf); //Returns a parsed structure cmdLine from a given strLine string
        //printCmdLine(parsedLine);
        execute(parsedLine);
        freeCmdLines(parsedLine);//Releases the memory that was allocated to accomodate the linked list of cmdLines
    }

    return 0;
} 

// Although you loop infinitely, the execution ends after execv. Why is that? 

// The exec family of functions do not kill your process. They replace the existing process image with the one you execute!!!
// Basically, it works as if, that process (with its PID and associated kernel resources) remains the same, 
// except all the code from the old image is removed and replaced by the code from the program that is then
// loaded into memory and initialized as if it were a new process altogether. The PID does not change, 
// so if you want to create a child process with its own PID, you have to use another function.
// The correct way to proceed is to fork first, and use exec* from the child process. 
// This way you can use the wait function in the parent instance to wait for the child to terminate and take control back.


// You must place the full path of an executable file in-order to run properly. 
// For instance: "ls" won't work, whereas "/bin/ls" runs properly. (Why?) 

//problem with the input/output redirect

//Wildcards, as in "ls *", are not working. (Again, why?) 
//Wildcard processing can be expensive because it requires to browse a folder. It is normally active by default in the shell, but not in API functions.
