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
#define SLEEP_TIME 3

//flag is set to true if signal has been cought
bool flag = false;

void mySignalHandler(int snum){
	printf("\nHandlig signal snum = %d in prosses...\n", snum);
	printf("Done!\n");
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
		perror("Coulnd't fork child #1\n");
		exit(1);
	}
	if (child_1 == 0){
		printf("child #1 %d\n", getpid());
		sleep(SLEEP_TIME);
		if (flag){
			close(fd[0]);
			if (write(fd[1], "Hello, my parent! From child #1\n", MESSAGE_SIZE) > 0)
					printf("Child #1 sent his first greeting\n");
			}
		return 0;
		}
	if (child_1 > 0){
		child_2 = fork();
		if(child_2 == -1){
			perror("Coulnd't fork child #2\n");
			exit(1);
	        }
	        if (child_2 == 0){
	        sleep(SLEEP_TIME);
	        if (flag){
	        	close(fd[0]);
			if (write(fd[1], "Hello, my parent! From child #2\n", MESSAGE_SIZE) > 0)
					printf("Child #2 sent his first greeting\n");
			}
		
			
		return 0;
	}else{
	        	
	        	printf("Parent's waiting for Ctrl+C being pressed to send messages from children\n");
		 	sleep(SLEEP_TIME);
		 	
		 	if (flag){
		 		close(fd[1]);
		 		char msg1[MESSAGE_SIZE];
		 		char msg2[MESSAGE_SIZE];
		 		//writing in pipe if we cought the signal
				if (read(fd[0], msg1, MESSAGE_SIZE) > 0)
					printf("Parent read from his child %s\n", msg1);
				if (read(fd[0], msg2, MESSAGE_SIZE) > 0)
					printf("Parent read from his child %s\n", msg2);
		 	}
		 	
			pid_t child_pid;
			int status;
			 			
			//waiting for the second child to finish
			child_pid = wait(&status);
			if (WIFEXITED(status))
			 	printf("Parent: child with pid = %d finished with code %d\n",
			 		child_pid, WEXITSTATUS(status) );
			else if (WIFSTOPPED(status))
			 	printf("Parent: child %d finished with code %d\n",
			 		child_pid, WSTOPSIG(status) );
			 		 
			 //waiting for the first child to finish	 
			 child_pid = wait(&status);
			 if (WIFEXITED(status))
			 	printf("Parent: child with pid = %d finished with code %d\n",
			 		child_pid, WEXITSTATUS(status) );
			 else if (WIFSTOPPED(status))
			 	printf("Parent: child %d finished with code %d\n",
			 		child_pid, WSTOPSIG(status) );
		 return 0;
	        }
	}
	}
	
	


