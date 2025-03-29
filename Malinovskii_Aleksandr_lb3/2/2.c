#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

volatile sig_atomic_t sigint_count = 0;

// Обработчик для режима process_single (однократный перехват)
void process_handler_single(int signum) {
    printf("Процесс: получен SIGINT (однократно).\n");
    signal(SIGINT, SIG_DFL);
}

// Обработчик для режима process_multi (многократный перехват)
void process_handler_multi(int signum) {
    sigint_count++;
    printf("Процесс: получен SIGINT %d раз(а).\n", sigint_count);
    if (sigint_count >= 3) {
        printf("Восстанавливаем исходный обработчик после %d срабатываний.\n", sigint_count);
        signal(SIGINT, SIG_DFL);
    }
}

// Функция потока для thread_single (однократное ожидание сигнала)
void* thread_signal_handler_single(void* arg) {
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigwait(&set, &sig);
    printf("Поток: получен SIGINT (однократно).\n");
    return NULL;
}

// Функция потока для thread_multi (многократное ожидание сигнала)
void* thread_signal_handler_multi(void* arg) {
    sigset_t set;
    int sig;
    int local_count = 0;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    while (local_count < 3) {
        sigwait(&set, &sig);
        local_count++;
        printf("Поток: получен SIGINT %d раз(а).\n", local_count);
    }
    printf("Поток: после %d срабатываний восстанавливаем исходное поведение.\n", local_count);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    return NULL;
}

// 2.5 Пример переназначения другой комбинации клавиш (Ctrl+Z)
void sigtstp_handler(int signum) {
    printf("Получен SIGTSTP (Ctrl+Z)!\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <mode>\n", argv[0]);
        fprintf(stderr, "Режимы: process_single, process_multi, thread_single, thread_multi, custom_signal\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "--process_single") == 0) {
        signal(SIGINT, process_handler_single);
        printf("Режим process_single. Нажмите Ctrl+C.\n");
        while (1) pause();
    }
    else if (strcmp(argv[1], "--process_multi") == 0) {
        signal(SIGINT, process_handler_multi);
        printf("Режим process_multi. Нажмите Ctrl+C несколько раз.\n");
        while (1) pause();
    }
    else if (strcmp(argv[1], "--thread_single") == 0) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, thread_signal_handler_single, NULL);
        printf("Режим thread_single. Нажмите Ctrl+C.\n");
        pthread_join(tid, NULL);
    }
    else if (strcmp(argv[1], "--thread_multi") == 0) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, thread_signal_handler_multi, NULL);
        printf("Режим thread_multi. Нажмите Ctrl+C несколько раз.\n");
        pthread_join(tid, NULL);
    }
    else if (strcmp(argv[1], "--custom_signal") == 0) {
        signal(SIGTSTP, sigtstp_handler);
        printf("Режим custom_signal. Нажмите Ctrl+Z.\n");
        while (1) pause();
    }
    else {
        fprintf(stderr, "Неизвестный режим: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    return 0;
}
