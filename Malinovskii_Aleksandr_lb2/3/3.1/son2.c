#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int main() {
    printf("Son2: Started (PID: %d)\n", getpid());
    signal(SIGINT, SIG_IGN); // Игнорируем SIGINT
    sleep(5); // Работаем 5 секунд
    printf("Son2: Exiting after 5 seconds.\n");
    exit(0);
}