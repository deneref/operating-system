#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define READER_BORDER "\t\t\t\t\t\t"

#define PERMS S_IRWXU | S_IRWXG | S_IRWXO

#define MAX_VALUE 10
#define WRITERS 3
#define READERS 5

#define ACTIVE_WR 0
#define ACTIVE_RR 1
#define WAITING_W 2
#define WAITING_R 3

#define V 1
#define P -1
#define Z 0

#define SEM_ACTIVE_WR 0
#define SEM_ACTIVE_RR 1
#define SEM_WAITING_W 2
#define SEM_WAITING_R 3

struct sembuf start_reading[] = {
    {SEM_WAITING_R, V, SEM_UNDO},
    {SEM_ACTIVE_WR, Z, SEM_UNDO},
    {SEM_WAITING_W, Z, SEM_UNDO},
    {SEM_ACTIVE_RR, V, SEM_UNDO},
    {SEM_WAITING_R, P, SEM_UNDO}
};
struct sembuf stop_read[] = {
    {SEM_ACTIVE_RR, P, SEM_UNDO}
};

struct sembuf start_writing[] = {
    {SEM_WAITING_W, V, SEM_UNDO},
    {SEM_ACTIVE_RR, Z, SEM_UNDO},
    {SEM_ACTIVE_WR, Z, SEM_UNDO},
    {SEM_ACTIVE_WR, V, SEM_UNDO},
    {SEM_WAITING_W, P, SEM_UNDO}
};
struct sembuf stop_write[] = {
    {SEM_ACTIVE_WR, P, SEM_UNDO}
};

int *shared_value;

pid_t *child_pids;


void kill_writers() {
    for (int i = 0; i < WRITERS; i++) {
        if (child_pids[i] == getpid()) {
            continue;
        }
        kill(child_pids[i], SIGTERM);
    }
}

int init_sh_mem() {
    int fd = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | PERMS);
    if (fd == -1) {
        perror("shmget");
        exit(1);
    }
    return fd;
}

int *get_sh_mem_addr(const int fd) {
    int *addr = (int *) shmat(fd, NULL, 0);
        if (addr == (int *) -1) {
        perror("shmat error");
        exit(1);
    }
    return addr;
}

int init_semaphores() {
    int semid = semget(IPC_PRIVATE, 4, IPC_CREAT | PERMS);
        if (semid == -1) {
        perror("semget error");
        exit(1);
    }

    int mem_ctrl    = semctl(semid, WAITING_R, SETVAL, 0);
    int writer_ctrl = semctl(semid, ACTIVE_WR, SETVAL, 0);
    int reader_ctrl = semctl(semid, ACTIVE_RR, SETVAL, 0);
    int wait_ctrl   = semctl(semid, WAITING_W, SETVAL, 0);

    if (mem_ctrl == -1 || writer_ctrl == -1 || reader_ctrl == -1 || wait_ctrl == -1) {
        perror("semctl");
        exit(1);
    }

    return semid;
}

void writer(int semid, int number) {
    int can = semop(semid, start_writing, 5);
    if (can == -1) {
        perror("semop start writing error");
        exit(1);
    }
    // Z condition for stopping all writers
    if (*shared_value >= MAX_VALUE) {
        kill_writers();
        int sem_op_stop = semop(semid, stop_write, 1);
        if (sem_op_stop == -1) {
            perror("semop stor write error");
            exit(1);
        }
        exit(0);
    }
    // write
    (*shared_value)++;
    printf("Writer #%d, pid=%d wrote value %d\n", number, getpid(), *shared_value);

    int sem_op_stop = semop(semid, stop_write, 1);
    if (sem_op_stop == -1) {
        perror("semop stop_write");
        exit(1);
    }

    sleep(rand() % 10);
}

void reader(int semid, int number) {
    int can = semop(semid, start_reading, 5);
    if (can == -1) {
        perror("semop start_reading");
        exit(1);
    }
    // read
    int val = *shared_value;
    printf(READER_BORDER"Reader #%d, pid=%d read value: %d\n", number, getpid(), val);

    int sem_op_stop = semop(semid, stop_read, 1);
    if (sem_op_stop == -1) {
        perror("semop stop_read");
        exit(1);
    }
    // Z condition for stopping all readers
    if (val >= MAX_VALUE) {
        exit(0);
    }

    sleep(rand() % 10);
}

void init_writer(int number, int sem_id) {
    pid_t pid;

    if ((pid = fork()) == -1) {
        printf("Can't fork");
        exit(1);
    }

    if (pid == 0) {
        printf("Writer #%d created, pid: %d\n", number, getpid());
        while (1) {
            writer(sem_id, number);
        }
    }
    else {
        child_pids[number] = pid;
    }
}

void init_reader(int number, const int sem_id) {
    pid_t pid;

    if ((pid = fork()) == -1) {
        printf("Can't fork");
        exit(1);
    }

    if (pid == 0) {
        printf(READER_BORDER"Reader #%d created, pid: %d\n", number, getpid());
        while (1) {
            reader(sem_id, number);
        }
    }
    else {
        child_pids[WRITERS + number] = pid;
    }
}

int main() {
    int sh_mem_fd = init_sh_mem();
    int *sh_mem = get_sh_mem_addr(sh_mem_fd);

    shared_value = sh_mem;
    *shared_value = 0;
    child_pids = shared_value + 1;

    int semid = init_semaphores();

    for (int i = 0; i < WRITERS; i++) {
        init_writer(i, semid);
    }

    for (int i = 0; i < READERS; i++) {
        init_reader(i, semid);
    }

    for (int i = 0; i < WRITERS + READERS; i++) {
        int *status;
        wait(status);
    }
}