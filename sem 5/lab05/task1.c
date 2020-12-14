#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <stdlib.h>

#define BUFFER_SIZE 10 
#define P -1
#define V 1

#define SE 0
#define SF 1
#define SB 2

char* shared_buffer;
int* pos_prod;
int* pos_cons;
int num = 0;

int semid = 1;

const int PERMISSIONS = S_IRWXU | S_IRWXG | S_IRWXO; //permition to read, write & execute by user, group & others
const int amount_of_consumers = 3; //amount of consumers
const int amount_of_producers = 3; //amount of producers

struct sembuf sembuf_producer_start[2] = {{SB, P, 0}, {SE, V, 0}}; 
struct sembuf sembuf_producer_stop[2] = {{SF, P, 0}, {SB, P, 0}};
struct sembuf sembuf_consumer_start[2] = {{SB, P, 0}, {SF, V, 0}};
struct sembuf sembuf_consumer_stop[2] = {{SE, P, 0}, {SB, P, 0}};

void produce(const int number){
	while(1){
		sleep(rand() % 2);
		
		if (semop(semid, sembuf_producer_start, 2) == -1){
			perror("Error semop producer start");
			exit(1);
		}
		
		printf("Producer %d produces %d \n", getpid(), *pos_prod);
		*(shared_buffer + *pos_prod) = *pos_prod;
		(*pos_prod)++;
		
		if (semop(semid, sembuf_producer_stop, 2) == -1){
			perror("Error semop producer stop");
			exit(1);
		}
	}
	
}

void consume(const int number){
	while(1){
		sleep(rand() % 2);
		
		if (semop(semid, sembuf_consumer_start, 2) == -1){
			perror("Error semop consumer start");
			exit(1);
		}
		
		printf("Consumer %d produces %d \n", getpid(), *pos_prod);
		*(shared_buffer + *pos_cons) = *pos_cons;
		(*pos_cons)++;
		
		if (semop(semid, sembuf_consumer_stop, 2) == -1){
			perror("Error semop consumer stop");
			exit(1);
		}
	}
}

void fork_children(const int numberOfChildren, void(*func)(const int)){
	for (int i = 0; i < numberOfChildren; ++i){
		const int pid = fork();
		
		if (pid == -1){
			perror(1);
			exit(1);
		}else if(pid == 0) {
			func(i); 
		}
		exit(1);
	}
}

void wait_children(const int numberOfChildren){
	for (int i = 0; i<numberOfChildren; ++i){
		int status;
		int child_pid = wait(&status);
		if (child_pid == -1){
			perror("Error child_pid");
			exit(1);
		}
		
		if (WIFEXITED(status)){
			printf("Procces %d returned %s status", child_pid, WEXITSTATUS(status));
		}
	}
}

int main(){
	int shmid, semid;
	
	printf("Parentpid %d\n", getpid());
	
	//creating shared memory area with size not less then our shared buffer
	shmid = shmget(IPC_PRIVATE, BUFFER_SIZE * sizeof(char), IPC_CREAT | PERMISSIONS);
	if (shmid == -1){
		perror("Coulderrornt create shared mem seg\n");
		exit(1);
	}
	//attaching shared memory onto our address space
	char* addr = shmat(semid, NULL, 0);
	if (addr == (void *) -1){
		perror("Couldnt attach shm\n");
		exit(1);
	}
	
	semid = semget(IPC_PRIVATE, 3, IPC_CREAT | PERMISSIONS);
	if (semid == -1){
		perror("An error occured while creating semathors\n");
		exit(1);
	}
	
	if (
		semctl(semid, SE, SETVAL, BUFFER_SIZE) == -1 ||
		semctl(semid, SF, SETVAL, 0) == -1 ||
		semctl(semid, SB, SETVAL, 1) == -1 )
	{ 
		perror("An error occured while semctl\n");
		exit(1);
	}
		
	fork_children(amount_of_producers, produce);
	fork_children(amount_of_consumers, consume);
	
	wait_children(amount_of_producers+amount_of_consumers);
	
	return 0;
}
