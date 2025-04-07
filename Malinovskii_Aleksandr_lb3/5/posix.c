/******************************************************************************
 * 
 * Цель:
 *  Демонстрация взаимодействия между двумя процессами (родительским и
 *  дочерним) через разделяемую память, синхронизированную семафорами.
 *  Родительский процесс пишет сообщения в разделяемую память, а дочерний
 *  процесс читает их.
 *
 * Описание:
 *  Программа создает разделяемую память и два семафора: sem_write и sem_read.
 *  Родительский процесс записывает сообщения в разделяемую память и
 *  уведомляет дочерний процесс об этом, увеличивая значение sem_read.
 *  Дочерний процесс ждет уведомления от родительского процесса, читает
 *  сообщение из разделяемой памяти и уведомляет родительский процесс,
 *  увеличивая значение sem_write.
 *
 * Особенности:
 *  - Используются функции shm_open, ftruncate, mmap для работы с
 *    разделяемой памятью.
 *  - Используются функции sem_open, sem_wait, sem_post, sem_close, sem_unlink
 *    для работы с семафорами.
 *  - В программе предусмотрена функция cleanup_resources для освобождения
 *    ресурсов в случае возникновения ошибок.
 *  - Для передачи данных используется структура shared_data, содержащая массив
 *    символов data.
 *
 * Замечания:
 *  - При запуске программы необходимо указать количество итераций в качестве
 *    аргумента командной строки.
 *  - Программа удаляет разделяемую память и семафоры после завершения работы.
 *
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define SHM_NAME "/my_shm"
#define SEM_WRITE "/sem_write"
#define SEM_READ  "/sem_read"
#define SHM_SIZE 1024

typedef struct {
    char data[SHM_SIZE];
} shared_data;

void print_usage(const char *prog_name) {
    fprintf(stderr, "Использование: %s <количество_итераций>\n", prog_name);
    fprintf(stderr, "Пример: %s 5\n", prog_name);
}

// Функция для очистки ресурсов (разделяемая память и семафоры)
void cleanup_resources(shared_data *shdata, size_t shdata_size, sem_t *sem_write, sem_t *sem_read) {
    if (sem_write != SEM_FAILED && sem_write != NULL) {
        sem_close(sem_write);
        sem_unlink(SEM_WRITE);
    }
    if (sem_read != SEM_FAILED && sem_read != NULL) {
        sem_close(sem_read);
        sem_unlink(SEM_READ);
    }
    if (shdata != MAP_FAILED && shdata != NULL) {
        munmap(shdata, shdata_size);
    }
    shm_unlink(SHM_NAME);
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

    // Удаляем возможные остатки от предыдущих запусков
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_WRITE);
    sem_unlink(SEM_READ);

    // Создаем разделяемую память
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    shared_data *shdata = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shdata == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    close(shm_fd); // Дескриптор больше не нужен

    // Создаем семафоры
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT, 0666, 1);
    if (sem_write == SEM_FAILED) {
        perror("sem_open (sem_write)");
        cleanup_resources(shdata, sizeof(shared_data), SEM_FAILED, SEM_FAILED);
        exit(EXIT_FAILURE);
    }
    sem_t *sem_read = sem_open(SEM_READ, O_CREAT, 0666, 0);
    if (sem_read == SEM_FAILED) {
        perror("sem_open (sem_read)");
        cleanup_resources(shdata, sizeof(shared_data), sem_write, SEM_FAILED);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        cleanup_resources(shdata, sizeof(shared_data), sem_write, sem_read);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) { 
        // Процесс-читатель (ребенок)
        for (int i = 0; i < iterations; i++) {
            if (sem_wait(sem_read) == -1) {
                perror("sem_wait (sem_read)");
                break;
            }
            printf("Читатель получил: %s\n", shdata->data);
            if (sem_post(sem_write) == -1) {
                perror("sem_post (sem_write)");
                break;
            }
        }
        sem_close(sem_write);
        sem_close(sem_read);
        munmap(shdata, sizeof(shared_data));
        exit(EXIT_SUCCESS);
    }
    else {
        // Процесс-писатель (родитель)
        for (int i = 0; i < iterations; i++) {
            if (sem_wait(sem_write) == -1) {
                perror("sem_wait (sem_write)");
                break;
            }
            snprintf(shdata->data, SHM_SIZE, "Сообщение %d", i + 1);
            printf("Писатель отправил: %s\n", shdata->data);
            if (sem_post(sem_read) == -1) {
                perror("sem_post (sem_read)");
                break;
            }
        }
        wait(NULL);
        // Закрываем и удаляем ресурсы
        sem_close(sem_write);
        sem_close(sem_read);
        munmap(shdata, sizeof(shared_data));
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_WRITE);
        sem_unlink(SEM_READ);
    }

    return EXIT_SUCCESS;
}