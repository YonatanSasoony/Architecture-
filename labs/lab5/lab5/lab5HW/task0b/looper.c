#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


void handleSignals(int sig){
	printf("\nreceived: %s\n",strsignal(sig));
	signal(sig, SIG_DFL);
	raise(sig);
	if(sig == SIGCONT) signal(SIGTSTP ,handleSignals);
	if(sig == SIGTSTP) signal(SIGCONT ,handleSignals);
	//signal(sig  ,handleSignals);
}

int main(int argc, char **argv){ 

	printf("Starting the program\n");
	signal(SIGTSTP ,handleSignals);
	signal(SIGINT  ,handleSignals);
	signal(SIGCONT ,handleSignals);

	while(1) {
		sleep(2);
	}

	return 0;
}
