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

#define STACK_SIZE (1024 * 1024) // 1 МБ стека для каждого потока
#define NUM_THREADS 3

volatile sig_atomic_t stop_flag = 0; // Флаг для остановки потоков

// Обработчик сигналов
void handle_signal(int sig) {
    stop_flag = 1;
    printf("Received signal %d, stopping threads...\n", sig);
}

// Функция для потоков, созданных через clone()
int run_clone_thread(void *arg) {
    int id = *(int *)arg;
    while (!stop_flag) {
        printf("Clone thread %d (TID: %ld)\n", id, (long)syscall(SYS_gettid));
        sleep(1);
    }
    printf("Clone thread %d exiting\n", id);
    return 0;
}

// Функция для потоков, созданных через pthread
void *run_pthread_thread(void *arg) {
    int id = *(int *)arg;
    while (!stop_flag) {
        printf("Pthread thread %d (TID: %ld)\n", id, (long)syscall(SYS_gettid));
        sleep(1);
    }
    printf("Pthread thread %d exiting\n", id);
    return NULL;
}

int main(void) {
    // Настройка обработчика сигналов
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    pthread_t pthreads[NUM_THREADS];
    int thread_ids[NUM_THREADS * 2]; // Идентификаторы для всех потоков
    char *stacks[NUM_THREADS];
    pid_t clone_pids[NUM_THREADS];

    // Инициализация идентификаторов
    for (int i = 0; i < NUM_THREADS * 2; i++) {
        thread_ids[i] = i + 1;
    }

    // Создание потоков через pthread_create()
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&pthreads[i], NULL, run_pthread_thread, &thread_ids[i]) != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Создание потоков через clone()
    for (int i = 0; i < NUM_THREADS; i++) {
        stacks[i] = malloc(STACK_SIZE);
        if (!stacks[i]) {
            perror("malloc failed");
            // Освобождаем память, выделенную для предыдущих стеков
            for (int j = 0; j < i; j++) {
                free(stacks[j]);
            }
            exit(EXIT_FAILURE);
        }

        clone_pids[i] = clone(run_clone_thread,
                              stacks[i] + STACK_SIZE,
                              CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM,
                              &thread_ids[i + NUM_THREADS]);

        if (clone_pids[i] == -1) {
            fprintf(stderr, "clone failed: %s\n", strerror(errno));
            // Освобождаем память, выделенную для стеков
            for (int j = 0; j <= i; j++) {
                free(stacks[j]);
            }
            exit(EXIT_FAILURE);
        }
    }

    // Ожидание сигнала для завершения
    while (!stop_flag) {
        pause();
    }

    // Ожидание завершения pthread потоков
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(pthreads[i], NULL) != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(errno));
        }
    }

    // Ожидание завершения clone потоков
    for (int i = 0; i < NUM_THREADS; i++) {
        if (waitpid(clone_pids[i], NULL, 0) == -1) {
            fprintf(stderr, "waitpid failed: %s\n", strerror(errno));
        }
        free(stacks[i]);
    }

    printf("All threads exited\n");
    return EXIT_SUCCESS;
}