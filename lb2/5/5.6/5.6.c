/**
 * 5.6.c
 *
 * Функциональность:
 * 1. Создание и управление потоками с разными политиками планирования:
 *    - SCHED_OTHER (стандартное планирование)
 *    - SCHED_RR (Round Robin)
 *    - SCHED_FIFO (First In First Out)
 *
 * 2. Ключевые особенности:
 *    - Установка индивидуальных параметров для каждого потока
 *    - Поддержка изменения приоритетов через аргументы командной строки
 *    - Детальный вывод информации о параметрах потоков
 *    - CPU-intensive нагрузка для демонстрации планирования
 *
 * 3. Технические детали:
 *    - Использование pthread_attr_t для настройки потоков
 *    - Явное указание наследования параметров (PTHREAD_EXPLICIT_SCHED)
 *    - Проверка и обработка ошибок для всех системных вызовов
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

#define NUM_THREADS 3

typedef struct {
    const char* name;
    int policy;
    int priority;
} thread_info;

void print_thread_info(const char* thread_name) {
    int policy;
    struct sched_param param;
    if (pthread_getschedparam(pthread_self(), &policy, &param) != 0) {
        perror("pthread_getschedparam failed");
        return;
    }

    const char* policy_name;
    switch (policy) {
        case SCHED_FIFO: policy_name = "SCHED_FIFO"; break;
        case SCHED_RR: policy_name = "SCHED_RR"; break;
        case SCHED_OTHER: policy_name = "SCHED_OTHER"; break;
        default: policy_name = "UNKNOWN";
    }

    printf("| %s | TID: %lu | Policy: %s | Priority: %d\n", 
           thread_name, (unsigned long)pthread_self(), policy_name, param.sched_priority);
    fflush(stdout);
}

void* thread_func(void* arg) {
    thread_info* info = (thread_info*) arg;
    while (1) {
        volatile unsigned long dummy = 0;
        for (unsigned long j = 0; j < 1000000000UL; j++) {
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

    int policies[NUM_THREADS] = {SCHED_OTHER, SCHED_RR, SCHED_FIFO};

    // Получаем минимальный и максимальный приоритеты
    int min_fifo = sched_get_priority_min(SCHED_FIFO);
    int max_fifo = sched_get_priority_max(SCHED_FIFO);
    int min_rr = sched_get_priority_min(SCHED_RR);
    int max_rr = sched_get_priority_max(SCHED_RR);

    // Устанавливаем приоритеты по умолчанию (средние значения)
    int fifo_priority = (min_fifo + max_fifo) / 2;
    int rr_priority = (min_rr + max_rr) / 2;

    // Если переданы аргументы, задаём приоритеты
    if (argc > 2) {
        fifo_priority = atoi(argv[1]);
        rr_priority = atoi(argv[2]);

        // Проверяем границы значений
        if (fifo_priority < min_fifo || fifo_priority > max_fifo) {
            printf("Ошибка: приоритет FIFO должен быть в диапазоне [%d, %d]\n", min_fifo, max_fifo);
            return 1;
        }
        if (rr_priority < min_rr || rr_priority > max_rr) {
            printf("Ошибка: приоритет RR должен быть в диапазоне [%d, %d]\n", min_rr, max_rr);
            return 1;
        }
    } else {
        printf("Использование: %s <FIFO_priority> <RR_priority>\n", argv[0]);
        printf("По умолчанию FIFO=%d, RR=%d\n", fifo_priority, rr_priority);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        switch(i) {
            case 0: thread_data[i].name = "Thread_SCHED_OTHER"; break;
            case 1: thread_data[i].name = "Thread_SCHED_RR"; break;
            case 2: thread_data[i].name = "Thread_SCHED_FIFO"; break;
        }
        thread_data[i].policy = policies[i];

        pthread_attr_t attr;
        ret = pthread_attr_init(&attr);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_init: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }

        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setinheritsched: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }

        ret = pthread_attr_setschedpolicy(&attr, thread_data[i].policy);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setschedpolicy для %s: %s\n", 
                    thread_data[i].name, strerror(ret));
        }

        struct sched_param param;
        if (thread_data[i].policy == SCHED_FIFO) {
            param.sched_priority = fifo_priority;
        } else if (thread_data[i].policy == SCHED_RR) {
            param.sched_priority = rr_priority;
        } else {
            param.sched_priority = 0;
        }

        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret != 0) {
            fprintf(stderr, "Ошибка pthread_attr_setschedparam для %s: %s\n", 
                    thread_data[i].name, strerror(ret));
        }
        thread_data[i].priority = param.sched_priority;

        ret = pthread_create(&threads[i], &attr, thread_func, &thread_data[i]);
        if (ret != 0) {
            fprintf(stderr, "Ошибка создания потока %s: %s\n", 
                    thread_data[i].name, strerror(ret));
            exit(EXIT_FAILURE);
        }
        pthread_attr_destroy(&attr);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
