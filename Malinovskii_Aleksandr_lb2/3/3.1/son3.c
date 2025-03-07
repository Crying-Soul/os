#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void handle_signal(int sig) {
    printf("Son3: Received signal. still work%d\n", sig);
}

int main() {
    printf("Son3: Started (PID: %d)\n", getpid());
    signal(SIGINT, handle_signal); // Устанавливаем обработчик для SIGINT
    sleep(2); // Работаем 2 секунд после получения сигнала
    printf("Son3: Exiting after 2 seconds.\n");
    exit(0);
}