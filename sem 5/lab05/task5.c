/*
DIY sig handler
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#define MESSAGE_SIZE 64

//flag is set to true if signal has been cought
bool flag = false;

void mySignalHandler(int snum){
	printf("\n\nHandlig signal snum = %d in prosses...\n", snum);
	printf("Done!\n\n");
	flag = true;
}

int main(){
	int child_1, child_2;
	signal(SIGINT, mySignalHandler);
	
	//initializing pipe
	int fd[2];
	if (pipe(fd) == -1){
		printf("Coundn't create a pipe\n");
		exit(1);
	}
	
	child_1 = fork();
	if(child_1 == -1){
		perror("Coulnd't fork child #1");
		exit(1);
	}
	if (child_1 == 0){
		close(fd[0]);
		if (write(fd[1], "Hello from child #1, parent!\n", MESSAGE_SIZE) > 0)
			printf("Child #1 sent a greeting to the parent\n");
		 return 0;
	}
	if (child_1 > 0){
		child_2 = fork();
		if(child_2 == -1){
			perror("Coulnd't fork child #2");
			exit(1);
	        }
	        if (child_2 == 0){
	        	close(fd[0]);
		if (write(fd[1], "Hello from child #2, parent!\n", MESSAGE_SIZE) > 0)
			printf("Child #2 sent a greeting to the parent\n");
		 	return 0;
	        }else{
	        	char msg1[MESSAGE_SIZE], msg2[MESSAGE_SIZE];
	        	printf("\nParent's waiting for Ctrl+C being pressed to read messages from children\n");
		 	sleep(5);
		 	
		 	if (flag){
		 		//reading from pipe if we cought the signal
				close(fd[1]);
				read(fd[0], msg1, MESSAGE_SIZE);
				read(fd[0], msg2, MESSAGE_SIZE);
				
		 	}else{
		 		//setting custom message if there was no signal
		 		close(fd[1]);
		 		strcpy(msg1, "nothing! Parent waited for signal until timeout");
		 		strcpy(msg2, "nothing! Parent waited for signal until timeout");
		 	}
				printf("Parent read this from his first child: %s\n", msg1); 
				printf("Parent read this from his second child: %s\n", msg2); 
				
			  	pid_t child_pid;
			 	int status;
			 			
			 	//waiting for the second child to finish
			 	child_pid = wait(&status);
			 	if (WIFEXITED(status))
			 		printf("\nParent: child with pid = %d finished with code %d\n",
			 		 child_pid, WEXITSTATUS(status) );
			 	else if (WIFSTOPPED(status))
			 		printf("\nParent: child %d finished with code %d\n",
			 		 child_pid, WSTOPSIG(status) );
			 		 
			 	//waiting for the first child to finish	 
			 	child_pid = wait(&status);
			 	if (WIFEXITED(status))
			 		printf("\nParent: child with pid = %d finished with code %d\n",
			 		 child_pid, WEXITSTATUS(status) );
			 	else if (WIFSTOPPED(status))
			 		printf("\nParent: child %d finished with code %d\n",
			 		 child_pid, WSTOPSIG(status) );
		 return 0;
	        }
	}
	
	
}


