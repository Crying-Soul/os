#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// Функция для получения строкового представления политики планирования
const char* get_policy_name(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "Unknown";
    }
}

// Функция, выполняемая каждым потоком
void* thread_function(void* arg) {
    int thread_num = *((int*)arg);
    int policy;
    struct sched_param param;

    // Получаем текущую политику и приоритет потока
    pthread_getschedparam(pthread_self(), &policy, &param);

    // Выводим информацию о потоке
    printf("Thread %d started:\n", thread_num);
    printf("  - Policy: %s\n", get_policy_name(policy));
    printf("  - Priority: %d\n", param.sched_priority);
    printf("  - Running on CPU: %d\n", sched_getcpu());

    // Имитация работы потока
    for (int i = 0; i < 5; i++) {
        printf("Thread %d: Working (%d/5)\n", thread_num, i + 1);
    }

    printf("Thread %d finished.\n", thread_num);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_nums[3] = {1, 2, 3};
    struct sched_param param;

    printf("Main thread started.\n");

    // Создаем три потока
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_nums[i]);
        printf("Main thread: Created thread %d with ID %lu\n", thread_nums[i], threads[i]);
    }

    // Изменяем политику планирования для первого потока на SCHED_FIFO
    param.sched_priority = 50;
    pthread_setschedparam(threads[0], SCHED_FIFO, &param);
    printf("Main thread: Changed policy of thread 1 to SCHED_FIFO with priority %d\n", param.sched_priority);

    // Изменяем политику планирования для второго потока на SCHED_RR
    param.sched_priority = 50;
    pthread_setschedparam(threads[1], SCHED_RR, &param);
    printf("Main thread: Changed policy of thread 2 to SCHED_RR with priority %d\n", param.sched_priority);

    // Третий поток остается с политикой SCHED_OTHER (по умолчанию)
    printf("Main thread: Thread 3 remains with default policy (SCHED_OTHER)\n");

    // Ждем завершения всех потоков
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
        printf("Main thread: Thread %d joined.\n", thread_nums[i]);
    }

    printf("Main thread: All threads have finished execution.\n");

    return 0;
}