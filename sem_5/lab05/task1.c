#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define SEM_BIN   0
#define SEM_EMPTY 1
#define SEM_FULL  2

#define P -1
#define V 1

#define PRODUCERS_COUNT 3
#define CONSUMERS_COUNT 3

#define BUFFER_SIZE 3

#define PERMS S_IRWXU | S_IRWXG | S_IRWXO //permition to read, write & execute by user, group & others

#define COMSUMER_BORDER "\t\t\t\t\t\t"

int sem_id = -1;
int shm_id = -1;

int *shm = NULL;
int *shm_pos = NULL;
int *produced_value = NULL;
int *last_consumed = NULL;

struct sembuf producer_start[2] = {
    {SEM_EMPTY, P, SEM_UNDO},
    {SEM_BIN,   P, SEM_UNDO}
};
struct sembuf producer_stop[2] = {
    {SEM_BIN,   V, SEM_UNDO},
    {SEM_FULL,  V, SEM_UNDO}
};

struct sembuf consumer_start[2] = {
    {SEM_FULL,  P, SEM_UNDO},
    {SEM_BIN,   P, SEM_UNDO}
};
struct sembuf consumer_stop[2] = {
    {SEM_BIN,   V, SEM_UNDO},
    {SEM_EMPTY, V, SEM_UNDO}
};

void fork_children(const int n, void (*func)(const int)) {
    for (int i = 0; i < n; ++i) {
        const pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            if (func) {
                func(i);
            }
            exit(1);
        }
    }
}

void wait_children(const int n) {
    for (int i = 0; i < n; ++i) {
        int status;
        const pid_t child_pid = wait(&status);
        if (child_pid == -1) {
            perror("wait error");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("Process %d returns %d\n", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d terminated with signal %d\n", child_pid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Process %d stopped due signal %d\n", child_pid, WSTOPSIG(status));
        }
    }
}

void producer(const int id) {
    while(1) {
        sleep(rand() % 3);
        
        if(*produced_value > 122)
        	exit(0);
            
        if (semop(sem_id, producer_start, 2) == -1) {
            perror("semop");
            exit(1);
        }

	// write next value in shared memory
	*(shm + *shm_pos) = *produced_value;
	printf("Producer %d (pid %d) produces %c\n", id, getpid(), *produced_value);
	(*shm_pos)++;
	(*produced_value)++;

	if (semop(sem_id, producer_stop, 2) == -1) {
		perror("semop");
		exit(1);
	}

    }
}

void consumer(const int id) {
    while(1) {
        sleep(rand() % 2);
        
        if (*last_consumed >= 122)
        	exit(0);
        
        if (semop(sem_id, consumer_start, 2) == -1) {
            perror("semop");
            exit(1);
        }
        
	printf(COMSUMER_BORDER"Consumer %d (pid %d) consumes %c\n", id, getpid(), *(shm +(*shm_pos)-1));
	*(last_consumed) = *(shm +(*shm_pos)-1);
	(*shm_pos)--;
	
	if (semop(sem_id, consumer_stop, 2) == -1) {
		perror("semop");
		exit(1);
	}
    }
}


void init_semaphores() {
    sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | PERMS);
    if (sem_id == -1) {
        perror("semget");
        exit(1);
    }
    if (semctl(sem_id, SEM_BIN,   SETVAL, 1) == -1 ||
        semctl(sem_id, SEM_EMPTY, SETVAL, BUFFER_SIZE) == -1 ||
        semctl(sem_id, SEM_FULL,  SETVAL, 0) == -1) {
        perror("semctl");
        exit(1);
    }
}

void init_shared_memory() {
    shm_id = shmget(IPC_PRIVATE, (BUFFER_SIZE+3) * sizeof(int), IPC_CREAT | PERMS);

    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    shm = shmat(shm_id, 0, 0);
    if (shm == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    shm_pos = shm;
    *shm_pos = 0;
    produced_value = shm+2;
    last_consumed = shm+1;

    *produced_value = 97;
    *last_consumed = 97;
    shm = shm + 3;

}

int main() {
    int children = 0;
    srand((unsigned int) time(NULL));

    init_semaphores();
    init_shared_memory();

    fork_children(PRODUCERS_COUNT, producer);
    fork_children(CONSUMERS_COUNT, consumer);

    wait_children(PRODUCERS_COUNT + CONSUMERS_COUNT);

    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, SEM_BIN, IPC_RMID, 0);
}
