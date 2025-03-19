#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

#define NUM_THREADS 3

// Структура для передачи информации потоку
typedef struct {
    const char* name;
    int policy;
    int priority;
} thread_info;

// Функция для вывода информации о текущем потоке
void print_thread_info(const char* thread_name) {
    int policy;
    struct sched_param param;
    if (pthread_getschedparam(pthread_self(), &policy, &param) != 0) {
        perror("pthread_getschedparam failed");
        return;
    }

    const char* policy_name;
    switch (policy) {
        case SCHED_FIFO:
            policy_name = "SCHED_FIFO";
            break;
        case SCHED_RR:
            policy_name = "SCHED_RR";
            break;
        case SCHED_OTHER:
            policy_name = "SCHED_OTHER";
            break;
        default:
            policy_name = "UNKNOWN";
    }

  printf(" %s  PID: %d  Policy: %s  Priority: %d \n", 
       thread_name, getpid(), policy_name, param.sched_priority);
}

// Функция, выполняемая каждым потоком
void* thread_func(void* arg) {
    thread_info* info = (thread_info*) arg;
    while (1) {
        // Имитация вычислительной нагрузки
        volatile unsigned long dummy = 0;
        for (unsigned long j = 0; j < 100000000UL; j++) {
            dummy++;
        }
        print_thread_info(info->name);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];
    thread_info thread_data[NUM_THREADS];
    int ret;

    // Определяем политики для каждого потока:
    // 0 - SCHED_OTHER, 1 - SCHED_RR, 2 - SCHED_FIFO
    int policies[NUM_THREADS] = {SCHED_OTHER, SCHED_RR, SCHED_FIFO};

    for (int i = 0; i < NUM_THREADS; i++) {
        // Задаём имена потоков
        switch(i) {
            case 0: thread_data[i].name = "Thread_SCHED_OTHER"; break;
            case 1: thread_data[i].name = "Thread_SCHED_RR"; break;
            case 2: thread_data[i].name = "Thread_SCHED_FIFO"; break;
            default: thread_data[i].name = "Thread";
        }
        thread_data[i].policy = policies[i];

        pthread_attr_t attr;
        ret = pthread_attr_init(&attr);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_init: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }

        // Устанавливаем, что параметры планирования задаются явно
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setinheritsched: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }

        // Устанавливаем нужную политику планирования для потока
        ret = pthread_attr_setschedpolicy(&attr, thread_data[i].policy);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setschedpolicy для %s: %s\n", 
                    thread_data[i].name, strerror(ret));
        }

        // Устанавливаем приоритет. Для real-time политик (SCHED_RR, SCHED_FIFO) выбираем среднее значение,
        // для SCHED_OTHER приоритет обычно игнорируется (будет равен 0)
        struct sched_param param;
        int min_priority = sched_get_priority_min(thread_data[i].policy);
        int max_priority = sched_get_priority_max(thread_data[i].policy);
        int priority = (min_priority + max_priority) / 2;
        param.sched_priority = priority;
        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setschedparam для %s: %s\n", 
                    thread_data[i].name, strerror(ret));
        }
        thread_data[i].priority = priority;


        // Создаем поток с заданными атрибутами
        ret = pthread_create(&threads[i], &attr, thread_func, &thread_data[i]);
        if (ret != 0) {
            fprintf(stderr, "Ошибка создания потока %s: %s\n", 
                    thread_data[i].name, strerror(ret));
            exit(EXIT_FAILURE);
        }
        pthread_attr_destroy(&attr);
    }

    // Потоки работают в бесконечном цикле, завершение программы производится вручную
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
