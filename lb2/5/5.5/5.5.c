#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// Функция для установки политики планирования и приоритета
void set_scheduling_policy(int policy, int priority) {
  struct sched_param param;
  param.sched_priority = priority;
  if (sched_setscheduler(0, policy, &param)) {
    perror("sched_setscheduler");
    exit(EXIT_FAILURE);
  }
}

// Функция, которую выполняют процессы
void worker(const char *name) {
  for (int i = 0; i < 100; i++) {
    printf("%s is running\n", name);
  }
}

int main() {
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {                          // Дочерний процесс
    set_scheduling_policy(SCHED_FIFO, 50); // Устанавливаем SCHED_FIFO
    worker("Child (SCHED_FIFO)");
  } else {                               // Родительский процесс
    set_scheduling_policy(SCHED_RR, 50); // Устанавливаем SCHED_RR
    worker("Parent (SCHED_RR)");
    wait(NULL); // Ожидаем завершения дочернего процесса
  }

  return 0;
}