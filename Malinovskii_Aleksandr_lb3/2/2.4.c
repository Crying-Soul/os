#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

int counter = 0;
const int MAX_SIGNALS = 2;

void* thread_func(void* arg) {
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    while (1) {
        sigwait(&set, &sig);
        printf("Поток перехватил SIGINT, раз: %d\n", ++counter);
        if (counter >= MAX_SIGNALS) {
            printf("Восстановление стандартного обработчика\n");
            signal(SIGINT, SIG_DFL);
            break;
        }
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
    printf("Нажмите Ctrl+C (2 раза — потом выход)\n");
    pthread_join(tid, NULL);
    while (1) pause();
    return 0;
}
