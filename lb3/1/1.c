#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define INITIAL_DELAY 1    // 1 сек перед первым сигналом
#define OPERATION_TIME 3   // 3 сек работы до завершения
#define TIME_BUF_SIZE 30   // Размер буфера для времени
#define BUSY_LOOP_ITERATIONS 100000000 // Количество итераций "работы"

static pthread_t t1, t2;
static volatile sig_atomic_t second_thread_active = true;

// Функция для получения текущего времени с наносекундной точностью
static void get_current_time(char* buffer, size_t size) {
    struct timespec ts;
    struct tm tm_info;
    
    if (clock_gettime(CLOCK_REALTIME, &ts)) {
        perror("clock_gettime");
        strncpy(buffer, "00:00:00.000000000", size);
        return;
    }
    
    if (localtime_r(&ts.tv_sec, &tm_info) == NULL) {
        perror("localtime_r");
        strncpy(buffer, "00:00:00.000000000", size);
        return;
    }
    
    if (strftime(buffer, size, "%H:%M:%S", &tm_info) == 0) {
        perror("strftime");
        strncpy(buffer, "00:00:00", size);
    }
    
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%09ld", ts.tv_nsec);
}

// Обработчик сигнала для задания 1.1
static void sigusr1_handler_default(int sig) {
    char time_buf[TIME_BUF_SIZE];
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 2: Received SIGUSR1 signal (default handler)\n", time_buf);
}

// Обработчик сигнала для задания 1.2
static void sigusr1_handler_custom(int sig) {
    char time_buf[TIME_BUF_SIZE];
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 2: Received SIGUSR1 signal (custom handler)\n", time_buf);
    printf("[%s] Thread 2: Exiting via pthread_exit()\n", time_buf);
    pthread_exit(NULL);
}

// Функция второй нити
static void* thread2_func(void* arg) {
    char time_buf[TIME_BUF_SIZE];
    get_current_time(time_buf, sizeof(time_buf));
    
    // Определяем, какой обработчик использовать (из аргумента)
    void (*handler)(int) = (void (*)(int))arg;
    
    // Настройка обработчика сигналов
    struct sigaction sa = {
        .sa_handler = handler,
        .sa_flags = 0
    };
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return NULL;
    }
    
    printf("[%s] Thread 2: Started (TID: %lu)\n", time_buf, (unsigned long)pthread_self());
    
    // Основной цикл работы
    while (second_thread_active) {
        get_current_time(time_buf, sizeof(time_buf));
        printf("[%s] Thread 2: Active\n", time_buf);
        
        // Имитация работы
        for (size_t i = 0; i < BUSY_LOOP_ITERATIONS; i++) {
            __asm__ __volatile__("" ::: "memory");
        }
    }
    
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 2: Exiting normally\n", time_buf);
    return NULL;
}

// Функция первой нити
static void* thread1_func(void* arg) {
    char time_buf[TIME_BUF_SIZE];
    get_current_time(time_buf, sizeof(time_buf));
    
    printf("[%s] Thread 1: Started (TID: %lu)\n", time_buf, (unsigned long)pthread_self());
    
    // Создание второй нити с передачей обработчика в качестве аргумента
    void (*handler)(int) = (void (*)(int))arg;
    if (pthread_create(&t2, NULL, thread2_func, (void*)handler) != 0) {
        perror("pthread_create");
        return NULL;
    }
    
    // Задержка перед первым сигналом
    sleep(INITIAL_DELAY);
    
    // Отправка первого сигнала
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 1: Sending first SIGUSR1 to Thread 2\n", time_buf);
    if (pthread_kill(t2, SIGUSR1)) {
        perror("pthread_kill");
    }
    
    // Точное ожидание OPERATION_TIME секунд
    struct timespec start, now;
    if (clock_gettime(CLOCK_MONOTONIC, &start)) {
        perror("clock_gettime");
        pthread_kill(t2, SIGTERM);
        pthread_join(t2, NULL);
        return NULL;
    }
    
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 1: Monitoring for %d seconds...\n", time_buf, OPERATION_TIME);
    
    do {
        if (clock_gettime(CLOCK_MONOTONIC, &now)) {
            perror("clock_gettime");
            break;
        }
    } while ((now.tv_sec - start.tv_sec) < OPERATION_TIME);
    
    // Отправка сигнала завершения
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 1: Sending termination signal to Thread 2\n", time_buf);
    second_thread_active = false;
    if (pthread_kill(t2, SIGUSR1)) {
        perror("pthread_kill");
    }
    
    // Ожидание завершения второй нити
    pthread_join(t2, NULL);
    
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Thread 1: Thread 2 terminated\n", time_buf);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    char time_buf[TIME_BUF_SIZE];
    get_current_time(time_buf, sizeof(time_buf));
    
    printf("[%s] Main thread: Started (PID: %d)\n", time_buf, getpid());
    
    // Выбор обработчика в зависимости от аргумента командной строки
    void (*handler)(int) = sigusr1_handler_default;
    if (argc > 1 && strcmp(argv[1], "--custom-handler") == 0) {
        handler = sigusr1_handler_custom;
        printf("[%s] Using custom signal handler with pthread_exit()\n", time_buf);
    } else {
        printf("[%s] Using default signal handler\n", time_buf);
    }
    
    // Создание первой нити с передачей обработчика
    if (pthread_create(&t1, NULL, thread1_func, (void*)handler)) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }
    
    // Ожидание завершения первой нити
    pthread_join(t1, NULL);
    
    get_current_time(time_buf, sizeof(time_buf));
    printf("[%s] Main thread: All threads completed\n", time_buf);
    return EXIT_SUCCESS;
}