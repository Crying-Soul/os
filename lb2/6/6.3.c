#define _GNU_SOURCE // Для clone()
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void signal_handler(int sig) {
  printf("Процесс (PID %d) получил сигнал %d\n", getpid(), sig);
  fflush(stdout);
}

// Тест наследования сигналов при вызове fork()
void test_signal_inheritance_fork(void) {
  printf("=== Тест наследования сигналов через fork() ===\n");
  fflush(stdout);

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  // Флаг SA_RESTART позволяет автоматически перезапустить прерванные системные
  // вызовы
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) { // Дочерний процесс
    sleep(1);
    printf("Потомок (PID %d): Отправляет SIGUSR1 себе\n", getpid());
    fflush(stdout);
    if (raise(SIGUSR1) != 0) {
      perror("raise");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  } else { // Родительский процесс
    sleep(2);
    printf("Родитель (PID %d): Отправляет SIGUSR1 себе\n", getpid());
    fflush(stdout);
    if (raise(SIGUSR1) != 0) {
      perror("raise");
      exit(EXIT_FAILURE);
    }
    wait(NULL);
  }

  printf("Завершён тест наследования сигналов через fork().\n\n");
  fflush(stdout);
}

// Тест наследования сигналов при вызове exec()
// Здесь дочерний процесс после fork() вызывает execl(), чтобы перезапустить
// программу с аргументом "exec", что позволяет проверить, что обработчик
// сигнала сбрасывается до значения по умолчанию (в обычном случае SIGUSR1
// завершит процесс).
void test_signal_inheritance_exec(char *prog_name) {
  printf("=== Тест наследования сигналов через exec() ===\n");
  fflush(stdout);

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) { // Дочерний процесс: выполняем exec()
    sleep(1);
    printf("Потомок (PID %d): Выполняет exec для запуска нового процесса\n",
           getpid());
    fflush(stdout);
    // Запускаем текущую программу с аргументом "exec"
    if (execl(prog_name, prog_name, "exec", NULL) == -1) {
      perror("execl");
      exit(EXIT_FAILURE);
    }
  } else {
    wait(NULL);
  }

  printf("Завершён тест наследования сигналов через exec().\n\n");
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  // Если программа запущена с аргументом "exec", это означает, что мы попали в
  // exec()-ветку.
  if (argc > 1 && strcmp(argv[1], "exec") == 0) {
    printf("Процесс после exec (PID %d): Попытка отправить SIGUSR1 себе\n",
           getpid());
    fflush(stdout);
    // После exec обработчик сигнала сбрасывается до значения по умолчанию,
    // поэтому вызов raise(SIGUSR1) приведёт к завершению процесса.
    if (raise(SIGUSR1) != 0) {
      perror("raise");
      exit(EXIT_FAILURE);
    }
    // Если процесс не завершился, выводим сообщение
    printf("Процесс (PID %d) не был завершён после raise(SIGUSR1)\n", getpid());
    exit(EXIT_SUCCESS);
  }

  // Запуск экспериментов
  test_signal_inheritance_fork();
  test_signal_inheritance_exec(argv[0]);

  return 0;
}
