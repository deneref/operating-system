/*
proc-orphans
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
	int child_1, child_2;
	child_1 = fork();
	if(child_1 == -1){
		perror("Coulnd't fork child #1");
		exit(1);
	}
	if (child_1 == 0){
		printf("Child #1: pid=%d; group=%d; ppid=%d\n",
		 getpid(), getpgrp(), getppid());
		 
		 return 0;
	}
	if (child_1 > 0){
		child_2 = fork();
		if(child_2 == -1){
			perror("Coulnd't fork child #2");
			exit(1);
	        }
	        if (child_2 == 0){
	        	printf("\nChild #2: pid=%d; group=%d; ppid=%d\n",
		  	 getpid(), getpgrp(), getppid());
		 
		 return 0;
	        }else{	
	       	printf("Parent: pid=%d; group=%d; ppid=%d\n",
		  	 getpid(), getpgrp(), getppid());
		 	
		 	pid_t child_pid;
		 	int status;
		 	
		 	child_pid = wait(&status);
		 	if (WIFEXITED(status))
		 		printf("Parent: child %d finished with code %d\n",
		 		 child_pid, WEXITSTATUS(status) );
		 	else if (WIFSTOPPED(status))
		 		printf("Parent: child %d finished with code %d\n",
		 		 child_pid, WSTOPSIG(status) );
		 		 
		 	child_pid = wait(&status);
		 	if (WIFEXITED(status))
		 		printf("Parent: child %d finished with code %d\n",
		 		 child_pid, WEXITSTATUS(status) );
		 	else if (WIFSTOPPED(status))
		 		printf("Parent: child %d finished with code %d\n",
		 		 child_pid, WSTOPSIG(status) );
		 		 
		 		 
		 return 0;
	        }
	}
	
}


