#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_NAME "/my_shm"
#define SEM_WRITE "/sem_write"
#define SEM_READ  "/sem_read"
#define SHM_SIZE 1024

typedef struct {
    char data[SHM_SIZE];
} shared_data;

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

    // Удаляем возможные остатки прошлых запусков
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_WRITE);
    sem_unlink(SEM_READ);

    // Создаем разделяемую память
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    
    shared_data *shdata = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shdata == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(shm_fd); // Дескриптор больше не нужен
    
    // Создаем семафоры
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT, 0666, 1);
    sem_t *sem_read  = sem_open(SEM_READ, O_CREAT, 0666, 0);
    
    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) { // Ребенок – читатель
        for (int i = 0; i < iterations; i++) {
            sem_wait(sem_read);
            printf("Читатель получил: %s\n", shdata->data);
            sem_post(sem_write);
        }
        // Очистка ресурсов
        sem_close(sem_write);
        sem_close(sem_read);
        munmap(shdata, sizeof(shared_data));
        exit(EXIT_SUCCESS);
    }
    else { // Родитель – писатель
        for (int i = 0; i < iterations; i++) {
            sem_wait(sem_write);
            snprintf(shdata->data, SHM_SIZE, "Сообщение %d", i+1);
            printf("Писатель отправил: %s\n", shdata->data);
            sem_post(sem_read);
        }
        wait(NULL);
        // Очистка
        sem_close(sem_write);
        sem_close(sem_read);
        sem_unlink(SEM_WRITE);
        sem_unlink(SEM_READ);
        munmap(shdata, sizeof(shared_data));
        shm_unlink(SHM_NAME);
    }
    
    return 0;
}