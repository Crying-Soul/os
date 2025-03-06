#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE 32

// Функция для получения текущего времени в формате "секунды:наносекунды"
void get_current_time(char* buffer, size_t size) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    snprintf(buffer, size, "%ld:%09ld", ts.tv_sec, ts.tv_nsec);
}

// Функция для получения политики планирования
const char* get_scheduling_policy(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "UNKNOWN";
    }
}

// Функция, выполняемая в потоке
void* thread_function(void* arg) {
    char timestamp[20];
    int policy;
    struct sched_param param;
    int iteration = 0;

    while (iteration < 5) {
        // Получаем текущее время
        get_current_time(timestamp, sizeof(timestamp));

        // Получаем политику планирования и параметры
        pthread_getschedparam(pthread_self(), &policy, &param);

        // Выводим подробную информацию с фиксированной шириной столбцов
        printf("%-7s | Time: %-14s | PID: %-3d | TID: %-4ld | Nice: %-1d | Policy: %-12s\n",
               "Thread", timestamp, getpid(), syscall(SYS_gettid), param.sched_priority, get_scheduling_policy(policy));

        iteration++;
        sleep(1);  // Задержка для наглядности
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Создаем два потока
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    // Основной процесс также выводит свою информацию
    char timestamp[20];
    int policy;
    struct sched_param param;
    int iteration = 0;

    while (iteration < 5) {
        // Получаем текущее время
        get_current_time(timestamp, sizeof(timestamp));

        // Получаем политику планирования и параметры
        policy = sched_getscheduler(getpid());
        sched_getparam(getpid(), &param);

        // Выводим подробную информацию с фиксированной шириной столбцов
        printf("%-7s | Time: %-14s | PID: %-3d | TID: %-4ld | Nice: %-1d | Policy: %-12s\n",
               "Main", timestamp, getpid(), syscall(SYS_gettid), param.sched_priority, get_scheduling_policy(policy));

        iteration++;
        sleep(1);  // Задержка для наглядности
    }

    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}