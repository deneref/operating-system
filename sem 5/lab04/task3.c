/*
proc-child calls exec()
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
	int child_1, child_2;
	child_1 = fork();
	if(child_1 == -1){
		perror("Coulnd't fork child #1");
		exit(1);
	}
	if (child_1 == 0){
		sleep(1);
		printf("\nChild #1 executes ps -al\n\n");
		if(execlp("ps", "ps", "-al", (char*)NULL) == -1){
			printf("Couldn't exec ps command\n");
			exit(1);
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
	        	printf("\nChild #2 executes ls -l\n\n");
		 	if(execlp("ls", "ls", "-l", (char*)NULL) == -1){
				printf("Couldn't exec ps command\n");
				exit(1);
			}
		 return 0;
	        }else{
	       	printf("\nParent: pid=%d; group=%d; ppid=%d\n",
		  	 getpid(), getpgrp(), getppid());
		  	 
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


