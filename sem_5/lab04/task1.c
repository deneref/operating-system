/*
proc-orphans
*/
#include <stdio.h>
#include <stdlib.h>

int main(){
	int child_1, child_2;
	child_1 = fork();
	if(child_1 == -1){
		perror("Coulnd't fork child #1");
		exit(1);
	}
	if (child_1 == 0){
		sleep(1);
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
	        	sleep(1);
	        	printf("\nChild #2: pid=%d; group=%d; ppid=%d\n",
		  	 getpid(), getpgrp(), getppid());
		 
		 return 0;
	        }else{
	       	printf("Parent: pid=%d; group=%d; ppid=%d\n",
		  	 getpid(), getpgrp(), getppid());
		 
		 return 0;
	        }
	}
	
	
}


