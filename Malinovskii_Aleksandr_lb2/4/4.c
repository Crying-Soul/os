#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define STACK_SIZE (1024 * 1024) // Размер стека для каждого потока (1 МБ)
#define NUM_THREADS 3            // Количество потоков каждого типа

volatile sig_atomic_t should_stop = 0; // Флаг для остановки потоков

// Обработчик сигналов
void signal_handler(int signal_number) {
    should_stop = 1; // Устанавливаем флаг остановки
    printf("Получен сигнал %d, завершение потоков...\n", signal_number);
}

// Функция для потоков, созданных через clone()
int clone_thread_function(void *thread_argument) {
    int thread_id = *(int *)thread_argument; // Получаем ID потока
    while (!should_stop) {
        printf("Поток clone %d (TID: %ld)\n", thread_id, (long)syscall(SYS_gettid));
        sleep(1);
    }
    printf("Поток clone %d завершен\n", thread_id);
    return 0;
}

// Функция для потоков, созданных через pthread
void *pthread_thread_function(void *thread_argument) {
    int thread_id = *(int *)thread_argument; // Получаем ID потока
    while (!should_stop) {
        printf("Поток pthread %d (TID: %ld)\n", thread_id, (long)syscall(SYS_gettid));
        sleep(1);
    }
    printf("Поток pthread %d завершен\n", thread_id);
    return NULL;
}

int main(void) {
    // Настройка обработчика сигналов
    struct sigaction signal_action;
    signal_action.sa_handler = signal_handler;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = 0;

    if (sigaction(SIGINT, &signal_action, NULL) == -1 || sigaction(SIGTERM, &signal_action, NULL) == -1) {
        perror("Ошибка настройки обработчика сигналов");
        exit(EXIT_FAILURE);
    }

    pthread_t pthread_handles[NUM_THREADS]; // Массив для хранения идентификаторов pthread
    int thread_ids[NUM_THREADS * 2];       // Идентификаторы для всех потоков
    char *thread_stacks[NUM_THREADS];      // Стеки для потоков, созданных через clone
    pid_t clone_thread_ids[NUM_THREADS];   // Идентификаторы потоков, созданных через clone

    // Инициализация идентификаторов потоков
    for (int i = 0; i < NUM_THREADS * 2; i++) {
        thread_ids[i] = i + 1;
    }

    // Создание потоков через pthread_create()
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&pthread_handles[i], NULL, pthread_thread_function, &thread_ids[i]) != 0) {
            fprintf(stderr, "Ошибка создания pthread: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Создание потоков через clone()
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_stacks[i] = malloc(STACK_SIZE); // Выделяем память для стека
        if (!thread_stacks[i]) {
            perror("Ошибка выделения памяти для стека");
            // Освобождаем память, выделенную для предыдущих стеков
            for (int j = 0; j < i; j++) {
                free(thread_stacks[j]);
            }
            exit(EXIT_FAILURE);
        }

        clone_thread_ids[i] = clone(clone_thread_function,
                                    thread_stacks[i] + STACK_SIZE, // Указатель на верх стека
                                    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM,
                                    &thread_ids[i + NUM_THREADS]);

        if (clone_thread_ids[i] == -1) {
            fprintf(stderr, "Ошибка создания clone: %s\n", strerror(errno));
            // Освобождаем память, выделенную для стеков
            for (int j = 0; j <= i; j++) {
                free(thread_stacks[j]);
            }
            exit(EXIT_FAILURE);
        }
    }

    // Ожидание сигнала для завершения
    while (!should_stop) {
        pause();
    }

    // Ожидание завершения потоков, созданных через pthread
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(pthread_handles[i], NULL) != 0) {
            fprintf(stderr, "Ошибка ожидания pthread: %s\n", strerror(errno));
        }
    }

    // Ожидание завершения потоков, созданных через clone
    for (int i = 0; i < NUM_THREADS; i++) {
        if (waitpid(clone_thread_ids[i], NULL, 0) == -1) {
            fprintf(stderr, "Ошибка ожидания clone: %s\n", strerror(errno));
        }
        free(thread_stacks[i]); // Освобождаем память стека
    }

    printf("Все потоки завершены\n");
    return EXIT_SUCCESS;
}