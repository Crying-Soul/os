#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

void* thread_func(void* arg) {
    while (1) {
    }
}

void handler(int signum) {
    printf("THREAD: Сигнал получен в потоке\n");
}

int main() {
    pthread_t th;
    pthread_create(&th, NULL, thread_func, NULL);
    signal(SIGINT, handler);
    printf("Основной поток ожидает Ctrl+C...\n");
    while (1) {

    }
    return 0;
}
