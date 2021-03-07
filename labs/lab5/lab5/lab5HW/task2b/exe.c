// void executeNextCommand(cmdLine *pCmdLine){
//         int pid = fork();
//         if(pid == -1) exit(1);
//         if(!pid){ //child code
//             if(debugMode) fprintf(stderr, "child PID: %d\n", pid);
//             execvp(pCmdLine->arguments[0], pCmdLine->arguments);
//             _exit(1);  
//         }//parent code
//         if(debugMode) fprintf(stderr,"parent PID: %d\n", pid);  
//         int wstatus;  
//         if(pCmdLine->blocking) waitpid(-1, &wstatus,0);
// }

// void execute(cmdLine *pCmdLine){
    
//     addProcess(&processList, pCmdLine, (pid_t)0);

//     if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
//         if(chdir(pCmdLine->arguments[1]))
//             fprintf(stderr,"change directory failed\n");
//     }
//     else if(pCmdLine->argCount == 1 && (!strncmp(pCmdLine->arguments[0],"procs",5) )){
//             printProcessList(&processList);
//     }
//     else         
//         executeNextCommand(pCmdLine);                    
// }

// void execute(cmdLine *pCmdLine){
    
//     addProcess(&processList, pCmdLine, (pid_t)0);

//     if(pCmdLine->argCount == 2 && (!strncmp(pCmdLine->arguments[0],"cd",2) )){
//         if(chdir(pCmdLine->arguments[1]))
//             fprintf(stderr,"change directory failed\n");
//     }
//     else if(pCmdLine->argCount == 1 && (!strncmp(pCmdLine->arguments[0],"procs",5) )){
//             printProcessList(&processList);
//     }
//     else         
//         executeNextCommand(pCmdLine);                    
// }