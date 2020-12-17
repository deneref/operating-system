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
		perror("Coulnd't fork child #1");
		exit(1);
	}
	if (child_1 == 0){
		close(fd[1]);
		sleep(SLEEP_TIME);
		if (flag){
			char msg[MESSAGE_SIZE];
			if (read(fd[0], msg, MESSAGE_SIZE) > 0){
				printf("Child #1 read from parent %s\n", msg);
			}
		}
		
		return 0;
	}
	if (child_1 > 0){
		child_2 = fork();
		if(child_2 == -1){
			perror("Coulnd't fork child #2");
			exit(1);
	        }
	        if (child_2 == 0){
	        	close(fd[1]);
	        
	        sleep(SLEEP_TIME);
	        if (flag){
			char msg[MESSAGE_SIZE];
			if (read(fd[0], msg, MESSAGE_SIZE) > 0){
				printf("Child #2 read from parent %s\n", msg);
			}
		}
			
		return 0;
	        }else{
	        	close(fd[0]);
	        	printf("Parent's waiting for Ctrl+C being pressed to send messages from children\n");
		 	sleep(SLEEP_TIME);
		 	
		 	if (flag){
		 		//writing in pipe if we cought the signal
				if (write(fd[1], "Hello, my child!\n", MESSAGE_SIZE) > 0)
					printf("Parent sent his first greeting\n");
				if (write(fd[1], "Hello again, my child!\n", MESSAGE_SIZE) > 0)
					printf("Parent sent his second greeting\n");
		 	}/*else{
		 		if (write(fd[1], "hoth\n", MESSAGE_SIZE) > 0)
					printf("Parent sent his first greeting\n");
				if (write(fd[1], "noth!\n", MESSAGE_SIZE) > 0)
					printf("Parent sent his second greeting\n");
		 	}*/
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


