#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_signal(int sig) {
  printf("Son3: Received signal. still work%d\n", sig);
}

int main() {
  printf("Son3: Started (PID: %d)\n", getpid());
  signal(SIGINT, handle_signal); // Устанавливаем обработчик для SIGINT
  sleep(5);                      // Работаем 5 секунд после получения сигнала
  printf("Son3: Exiting after 5 seconds.\n");
  exit(0);
}