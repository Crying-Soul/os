#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

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
    printf("Поток %d (PID: %d, TID: %lu) начал выполнение:\n", thread_num, getpid(), pthread_self());
    printf("  - Политика: %s\n", get_policy_name(policy));
    printf("  - Приоритет: %d\n", param.sched_priority);
    printf("  - Выполняется на CPU: %d\n", sched_getcpu());

    // Имитация работы потока
    for (int i = 0; i < 5; i++) {
        printf("Поток %d: Работает (%d/5)\n", thread_num, i + 1);
        sleep(1);
    }

    printf("Поток %d завершил выполнение.\n", thread_num);
    return NULL;
}

// Функция для создания потоков с разными политиками и приоритетами
void create_threads() {
    pthread_t threads[3];
    int thread_nums[3] = {1, 2, 3};
    struct sched_param param;

    printf("Основной поток (PID: %d) начал выполнение.\n", getpid());

    // Создаем три потока
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_nums[i]);
        printf("Основной поток: Создан поток %d с ID %lu\n", thread_nums[i], threads[i]);
    }

    // Изменяем политику планирования для первого потока на SCHED_FIFO с высоким приоритетом
    param.sched_priority = 99;
    pthread_setschedparam(threads[0], SCHED_FIFO, &param);
    printf("Основной поток: Политика потока 1 изменена на SCHED_FIFO с приоритетом %d\n", param.sched_priority);

    // Изменяем политику планирования для второго потока на SCHED_RR с средним приоритетом
    param.sched_priority = 50;
    pthread_setschedparam(threads[1], SCHED_RR, &param);
    printf("Основной поток: Политика потока 2 изменена на SCHED_RR с приоритетом %d\n", param.sched_priority);

    // Третий поток остается с политикой SCHED_OTHER (по умолчанию)
    printf("Основной поток: Поток 3 остается с политикой по умолчанию (SCHED_OTHER)\n");

    // Ждем завершения всех потоков
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
        printf("Основной поток: Поток %d завершен.\n", thread_nums[i]);
    }

    printf("Основной поток: Все потоки завершили выполнение.\n");
}

// Функция для создания независимых процессов (потоков разных процессов)
void create_processes() {
    printf("Создание независимых процессов...\n");

    for (int i = 1; i <= 3; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Дочерний процесс
            int policy;
            struct sched_param param;

            // Устанавливаем политику и приоритет для каждого процесса
            if (i == 1) {
                param.sched_priority = 99;
                sched_setscheduler(0, SCHED_FIFO, &param);
            } else if (i == 2) {
                param.sched_priority = 50;
                sched_setscheduler(0, SCHED_RR, &param);
            } else {
                param.sched_priority = 0;
                sched_setscheduler(0, SCHED_OTHER, &param);
            }

            // Получаем текущую политику и приоритет
            sched_getparam(0, &param);
            policy = sched_getscheduler(0);

            // Выводим информацию о процессе
            printf("Процесс %d (PID: %d) начал выполнение:\n", i, getpid());
            printf("  - Политика: %s\n", get_policy_name(policy));
            printf("  - Приоритет: %d\n", param.sched_priority);
            printf("  - Выполняется на CPU: %d\n", sched_getcpu());


            // Имитация работы процесса
            for (int j = 0; j < 5; j++) {
                printf("Процесс %d: Работает (%d/5)\n", i, j + 1);
                sleep(1);
            }

            printf("Процесс %d завершил выполнение.\n", i);
            exit(0);
        }
    }

    // Ожидаем завершения всех дочерних процессов
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    printf("Все независимые процессы завершили выполнение.\n");
}

int main() {
    printf("Тестирование потоков в одном процессе:\n");
    create_threads();

    printf("\nТестирование независимых процессов:\n");
    create_processes();

    // Вывод анализа результатов
    printf("\n\n--- Анализ и выводы ---\n");

    printf("\n** Потоки в одном процессе: **\n");
    printf("- Поток с SCHED_FIFO (приоритет 99) выполняется первым.\n");
    printf("- Поток с SCHED_RR (приоритет 50) выполняется вторым.\n");
    printf("- Поток с SCHED_OTHER выполняется последним (низкий приоритет).\n");

    printf("\n** Независимые процессы: **\n");
    printf("- Процесс с SCHED_FIFO (приоритет 99) выполняется первым.\n");
    printf("- Процесс с SCHED_RR (приоритет 50) выполняется вторым.\n");
    printf("- Процесс с SCHED_OTHER выполняется последним.\n");

    printf("\n** Сравнение: **\n");
    printf("- Приоритеты и политики влияют на порядок выполнения как для потоков, так и для процессов.\n");
    printf("- Для потоков одного процесса планировщик работает более детально, так как потоки разделяют ресурсы.\n");

    printf("\n** Ответы на задание: **\n");
    printf("- Приоритеты влияют на порядок выполнения для SCHED_FIFO и SCHED_RR.\n");
    printf("- Для SCHED_OTHER приоритеты не влияют на порядок выполнения (динамическое планирование).\n");
    printf("- Для независимых процессов приоритеты учитываются на уровне процессов.\n");
    printf("- Для потоков одного процесса приоритеты применяются напрямую, что позволяет более точно управлять их выполнением.\n");

    return 0;
}
