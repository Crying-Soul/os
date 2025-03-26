#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 1024

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void print_usage(const char *prog_name) {
    printf("Использование: %s <количество_итераций>\n", prog_name);
    printf("Пример: %s 5\n", prog_name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        printf("Количество итераций должно быть положительным числом\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Разделяемая память
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    // Семафоры
    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    union semun arg;
    unsigned short values[2] = {1, 0};  // sem_write=1, sem_read=0
    arg.array = values;
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) { // Ребенок (читатель)
        struct sembuf wait_read = {1, -1, 0};    // Ожидание sem_read
        struct sembuf signal_write = {0, 1, 0};  // Сигнал sem_write
        for (int i = 0; i < iterations; i++) {
            semop(semid, &wait_read, 1);
            printf("Читатель получил: %s\n", shm_ptr);
            semop(semid, &signal_write, 1);
        }
        shmdt(shm_ptr);
        exit(EXIT_SUCCESS);
    }
    else { // Родитель (писатель)
        struct sembuf wait_write = {0, -1, 0};   // Ожидание sem_write
        struct sembuf signal_read = {1, 1, 0};   // Сигнал sem_read
        for (int i = 0; i < iterations; i++) {
            semop(semid, &wait_write, 1);
            snprintf(shm_ptr, SHM_SIZE, "Сообщение %d", i+1);
            printf("Писатель отправил: %s\n", shm_ptr);
            semop(semid, &signal_read, 1);
        }
        wait(NULL);
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, 0);
    }
    
    return 0;
}