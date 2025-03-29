#define _GNU_SOURCE  // Для clone()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <sys/file.h>  // Для файловых блокировок

#define INITIAL_DELAY 1    // 1 сек перед первым сигналом
#define OPERATION_TIME 3   // 3 сек работы до завершения

pthread_t t1, t2;
volatile int second_thread_active = 1; // volatile для гарантии видимости изменений

// Функция для получения текущего времени с наносекундной точностью
void get_current_time(char* buffer) {
    struct timespec ts;
    struct tm tm_info;
    
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    strftime(buffer, 20, "%H:%M:%S", &tm_info);
    sprintf(buffer + strlen(buffer), ".%09ld", ts.tv_nsec);
}

// Обработчик сигнала
void sigusr1_handler(int sig) {
    char time_buf[30];
    get_current_time(time_buf);
    printf("[%s] Thread 2: Received SIGUSR1 signal\n", time_buf);
}

// Функция второй нити
void* thread2_func(void* arg) {
    char time_buf[30];
    get_current_time(time_buf);
    
    // Настройка обработчика сигналов
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return NULL;
    }
    
    printf("[%s] Thread 2: Started (TID: %lu)\n", time_buf, (unsigned long)pthread_self());
    
    // Основной цикл работы
    while (second_thread_active) {
        get_current_time(time_buf);
        printf("[%s] Thread 2: Active\n", time_buf);
        for (size_t i = 0; i < 100000000; i++)
        {

        }
        
    }
    
    get_current_time(time_buf);
    printf("[%s] Thread 2: Exiting\n", time_buf);
    return NULL;
}

// Функция первой нити
void* thread1_func(void* arg) {
    char time_buf[30];
    get_current_time(time_buf);
    
    printf("[%s] Thread 1: Started (TID: %lu)\n", time_buf, (unsigned long)pthread_self());
    
    // Создание второй нити
    if (pthread_create(&t2, NULL, thread2_func, NULL) != 0) {
        perror("pthread_create");
        return NULL;
    }
    
    // Задержка перед первым сигналом
    sleep(INITIAL_DELAY);
    
    // Отправка первого сигнала
    get_current_time(time_buf);
    printf("[%s] Thread 1: Sending first SIGUSR1 to Thread 2\n", time_buf);
    if (pthread_kill(t2, SIGUSR1) != 0) {
        perror("pthread_kill");
    }
    
    // Точное ожидание OPERATION_TIME секунд
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    get_current_time(time_buf);
    printf("[%s] Thread 1: Monitoring for %d seconds...\n", time_buf, OPERATION_TIME);
    
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while ((now.tv_sec - start.tv_sec) < OPERATION_TIME);
    
    // Отправка сигнала завершения
    get_current_time(time_buf);
    printf("[%s] Thread 1: Sending termination signal to Thread 2\n", time_buf);
    second_thread_active = 0;
    if (pthread_kill(t2, SIGUSR1) != 0) {
        perror("pthread_kill");
    }
    
    // Ожидание завершения второй нити
    pthread_join(t2, NULL);
    
    get_current_time(time_buf);
    printf("[%s] Thread 1: Thread 2 terminated\n", time_buf);
    
    return NULL;
}

int main() {
    char time_buf[30];
    get_current_time(time_buf);
    
    printf("[%s] Main thread: Started (PID: %d)\n", time_buf, getpid());
    
    // Создание первой нити
    if (pthread_create(&t1, NULL, thread1_func, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    // Ожидание завершения первой нити
    pthread_join(t1, NULL);
    
    get_current_time(time_buf);
    printf("[%s] Main thread: All threads completed\n", time_buf);
    return 0;
}