#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>

#define MAX_SIGNALS 10

_Atomic int total_signals = 0;
_Atomic int handler_active = 0;

void get_current_time(char* buffer) {
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    strftime(buffer, 20, "%H:%M:%S", &tm_info);
    sprintf(buffer + strlen(buffer), ".%09ld", ts.tv_nsec);
}

void cpu_intensive_operation(int ms) {
    volatile unsigned long iterations = ms * 100000UL;
    for (volatile unsigned long i = 0; i < iterations; i++) {}
}

void sigusr2_handler(int sig, siginfo_t *info, void *ucontext);

void sigusr1_handler(int sig, siginfo_t *info, void *ucontext) {
    char time_buf[30];
    get_current_time(time_buf);
    int current_count = atomic_fetch_add(&total_signals, 1) + 1;
    
    printf("[%s] Начало обработки SIGUSR1 (#%d)\n", time_buf, current_count);fflush(stdout);
    
    // Длительная обработка с циклами
    for (int i = 0; i < 5; i++) {
        cpu_intensive_operation(200);
        get_current_time(time_buf);
        printf("[%s] Обработка SIGUSR1... (%d/5)\n", time_buf, i+1);fflush(stdout);
        
        // В середине обработки отправляем SIGUSR2
        if (i == 2 && current_count < MAX_SIGNALS) {
            printf("[%s] >> Внутри обработки SIGUSR1: отправляю SIGUSR2\n", time_buf);fflush(stdout);
            kill(getpid(), SIGUSR2);
            cpu_intensive_operation(50);
        }
    }
    
    get_current_time(time_buf);
    printf("[%s] Конец обработки SIGUSR1 (#%d)\n", time_buf, current_count);
    fflush(stdout);
}

void sigusr2_handler(int sig, siginfo_t *info, void *ucontext) {
    char time_buf[30];
    get_current_time(time_buf);
    int current_count = atomic_fetch_add(&total_signals, 1) + 1;
    
    printf("[%s] Начало обработки SIGUSR2 (#%d)\n", time_buf, current_count);
    fflush(stdout);
    
    // Длительная обработка с циклами
    for (int i = 0; i < 5; i++) {
        cpu_intensive_operation(150);
        get_current_time(time_buf);
        printf("[%s] Обработка SIGUSR2... (%d/5)\n", time_buf, i+1);
        fflush(stdout);
        
        // В середине обработки отправляем SIGUSR1
        if (i == 3 && current_count < MAX_SIGNALS) {
            printf("[%s] >> Внутри обработки SIGUSR2: отправляю SIGUSR1\n", time_buf);
            fflush(stdout);
            kill(getpid(), SIGUSR1);
            cpu_intensive_operation(50);
        }
    }
    
    get_current_time(time_buf);
    printf("[%s] Конец обработки SIGUSR2 (#%d)\n", time_buf, current_count);
    fflush(stdout);
}

void setup_signal_handlers() {
    struct sigaction sa1, sa2;
    
    memset(&sa1, 0, sizeof(sa1));
    sa1.sa_sigaction = sigusr1_handler;
    sigfillset(&sa1.sa_mask);
    sa1.sa_flags = SA_SIGINFO;
    
    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_sigaction = sigusr2_handler;
    sigfillset(&sa2.sa_mask);
    sa2.sa_flags = SA_SIGINFO;
    
    if (sigaction(SIGUSR1, &sa1, NULL) == -1 || sigaction(SIGUSR2, &sa2, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main() {
    setup_signal_handlers();
    
    printf("Демонстрация взаимной генерации сигналов:\n");
    printf("- Каждый обработчик в середине работы отправляет другой сигнал\n");
    printf("- Максимальное количество сигналов: %d\n\n", MAX_SIGNALS);

    fflush(stdout);
    
    // Отправляем начальный сигнал
    kill(getpid(), SIGUSR1);
    
    // Ждем завершения обработки всех сигналов
    while (atomic_load(&total_signals) < MAX_SIGNALS) {
        cpu_intensive_operation(1000);
    }
    
    printf("\nИтог: обработано %d сигналов\n", atomic_load(&total_signals));
    fflush(stdout);
    return 0;
}
