/**************************************************************
 * Программа управления нитями с использованием SIGUSR1
 * 
 * Цели:
 * 1. Демонстрация принудительного завершения нитей через сигналы
 * 2. Реализация кастомного обработчика сигналов с pthread_exit()
 * 3. Точный контроль времени выполнения с использованием:
 *    - CLOCK_REALTIME для логирования
 *    - CLOCK_MONOTONIC для измерения интервалов
 * 
 * Особенности реализации:
 * - Использование pthread_kill() для управления состоянием нити
 * - Прямое завершение нити из обработчика сигналов
 * - Двойная отправка сигналов:
 *   * Первый сигнал - тестовый
 *   * Второй сигнал - для завершения
 * - Временные параметры:
 *   * INITIAL_DELAY = 1 сек (искусственная задержка)
 *   * OPERATION_TIME = 3 сек (время мониторинга)
 **************************************************************/

/*-------------------------------------------------------------
 * Ключевые функции:
 * 1. custom_signal_handler() - аварийное завершение нити
 * 2. thread2_func()          - бесконечный цикл с обработкой сигналов
 * 3. thread1_func()          - управление временем и сигналами
 * 
 * Важные особенности:
 * - Использование clock_gettime() двух типов:
 *   CLOCK_REALTIME для абсолютного времени в логах
 *   CLOCK_MONOTONIC для точного измерения интервалов
 * - Намеренная "засоренность" кода циклами for для демонстрации
 *   работы планировщика
 * - Неявная проблема: после завершения thread1_func вторая нить
 *   продолжит работать, если не получить сигнал повторно
 * 
 * Выводы:
 * 1. pthread_exit() в обработчике сигналов позволяет мгновенно
 *    завершить нить
 * 2. Точность CLOCK_MONOTONIC (±1 μs) достаточна для задач
 *    реального времени
 * 3. Логирование с наносекундным разрешением помогает анализировать
 *    параллельные процессы
 * 4. Необходимость двойной синхронизации: по сигналам и времени
 -------------------------------------------------------------*/

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define INITIAL_DELAY 1    // 1 сек перед первым сигналом
#define OPERATION_TIME 3   // 3 сек работы до завершения

pthread_t t1, t2;

// Функция для получения текущего времени
void get_current_time(char* buffer) {
    struct timespec ts;
    struct tm tm_info;
    
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    strftime(buffer, 20, "%H:%M:%S", &tm_info);
    sprintf(buffer + strlen(buffer), ".%09ld", ts.tv_nsec);
}

// НОВЫЙ обработчик сигнала с pthread_exit()
void custom_signal_handler(int sig) {
    char time_buf[30];
    get_current_time(time_buf);
    printf("[%s] Thread 2: Custom handler started - terminating thread\n", time_buf);
    pthread_exit(NULL);
}

// Функция второй нити
void* thread2_func(void* arg) {
    char time_buf[30];
    get_current_time(time_buf);
    
    // Настройка НОВОГО обработчика сигналов
    struct sigaction sa;
    sa.sa_handler = custom_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return NULL;
    }
    
    printf("[%s] Thread 2: Started (TID: %lu)\n", time_buf, (unsigned long)pthread_self());
    
    // Основной цикл работы
    while (1) {
        get_current_time(time_buf);
        printf("[%s] Thread 2: Active\n", time_buf);
        for (size_t i = 0; i < 100000000; i++)
        {

        }
    }
    
    // Этот код никогда не выполнится
    get_current_time(time_buf);
    printf("[%s] Thread 2: Normal exit\n", time_buf);
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
    for (size_t i = 0; i < 1000000000; i++)
        {

        }
    
    // Отправка первого сигнала (демонстрационный)
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
        get_current_time(time_buf);
        for (size_t i = 0; i < 100000000; i++){}
        printf("[%s] Thread 1: Still running\n", time_buf);
    } while ((now.tv_sec - start.tv_sec) < OPERATION_TIME);
    
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