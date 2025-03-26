#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 1024

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void print_usage(const char *prog_name) {
    fprintf(stderr, "Использование: %s <количество_итераций>\n", prog_name);
    fprintf(stderr, "Пример: %s 5\n", prog_name);
}

// Функция для очистки ресурсов (разделяемая память и семафоры)
void cleanup_resources(int shmid, char *shm_ptr, int semid) {
    if (shm_ptr != (char *)-1 && shm_ptr != NULL) {
        shmdt(shm_ptr);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "Количество итераций должно быть положительным числом\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Создание разделяемой памяти
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat");
        cleanup_resources(shmid, (char *)-1, -1);
        exit(EXIT_FAILURE);
    }

    // Создание семафоров
    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        cleanup_resources(shmid, shm_ptr, -1);
        exit(EXIT_FAILURE);
    }

    union semun arg;
    unsigned short values[2] = {1, 0};  // sem_write = 1, sem_read = 0
    arg.array = values;
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        cleanup_resources(shmid, shm_ptr, semid);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        cleanup_resources(shmid, shm_ptr, semid);
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) { // Читатель (ребенок)
        struct sembuf wait_read = {1, -1, 0};   // Ожидание sem_read
        struct sembuf signal_write = {0, 1, 0}; // Разрешить запись

        for (int i = 0; i < iterations; i++) {
            if (semop(semid, &wait_read, 1) == -1) {
                perror("semop (wait_read)");
                break;
            }
            printf("Читатель получил: %s\n", shm_ptr);
            if (semop(semid, &signal_write, 1) == -1) {
                perror("semop (signal_write)");
                break;
            }
        }

        shmdt(shm_ptr);
        exit(EXIT_SUCCESS);
    }
    else { // Писатель (родитель)
        struct sembuf wait_write = {0, -1, 0};  // Ожидание sem_write
        struct sembuf signal_read = {1, 1, 0};  // Разрешить чтение

        for (int i = 0; i < iterations; i++) {
            if (semop(semid, &wait_write, 1) == -1) {
                perror("semop (wait_write)");
                break;
            }
            snprintf(shm_ptr, SHM_SIZE, "Сообщение %d", i + 1);
            printf("Писатель отправил: %s\n", shm_ptr);
            if (semop(semid, &signal_read, 1) == -1) {
                perror("semop (signal_read)");
                break;
            }
        }

        wait(NULL);
        cleanup_resources(shmid, shm_ptr, semid);
    }

    return EXIT_SUCCESS;
}
