#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Вспомогательная функция для ожидания завершения дочернего процесса
void wait_for_child() {
  int status;
  if (wait(&status) < 0) {
    perror("wait failed");
    exit(EXIT_FAILURE);
  }
}

// Функция для демонстрации execl()
void run_execl(const char *path) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execl...\n", getpid());
    fflush(stdout);
    execl(path, "ls", "-l", "-a", (char *)NULL);
    perror("execl failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

// Функция для демонстрации execlp()
void run_execlp(const char *file) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execlp...\n", getpid());
    fflush(stdout);
    execlp(file, "ls", "-l", "-a", (char *)NULL);
    perror("execlp failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

// Функция для демонстрации execle() с передачей пользовательского окружения
void run_execle(const char *path, char *const envp[]) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execle (с пользовательским "
           "окружением)...\n",
           getpid());
    fflush(stdout);
    execle(path, "ls", "-l", "-a", (char *)NULL, envp);
    perror("execle failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

// Функция для демонстрации execv()
void run_execv(const char *path, char *const argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execv...\n", getpid());
    fflush(stdout);
    execv(path, argv);
    perror("execv failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

// Функция для демонстрации execvp()
void run_execvp(const char *file, char *const argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execvp...\n", getpid());
    fflush(stdout);
    execvp(file, argv);
    perror("execvp failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

// Функция для демонстрации execve() с передачей пользовательского окружения
void run_execve(const char *path, char *const argv[], char *const envp[]) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    printf("Дочерний процесс [%d] запускает execve (с пользовательским "
           "окружением)...\n",
           getpid());
    fflush(stdout);
    execve(path, argv, envp);
    perror("execve failed");
    exit(EXIT_FAILURE);
  }
  wait_for_child();
}

int main(void) {
  // Путь к исполняемому файлу и его псевдоним для поиска в PATH
  char *file = "ls";
  char *path = "/bin/ls";

  // Массив аргументов для запуска команды
  char *args[] = {"ls", "-l", "-a", NULL};

  // Массив переменных окружения для некоторых вызовов exec*
  char *env[] = {"MY_VAR=123", "PATH=/usr/bin:/bin", NULL};

  printf("Демонстрация работы различных функций exec():\n");
  fflush(stdout);
  printf("\nЗапуск execl:\n");
  fflush(stdout);
  run_execl(path);

  printf("\nЗапуск execlp:\n");
  fflush(stdout);
  run_execlp(file);

  printf("\nЗапуск execle (с пользовательским окружением):\n");
  fflush(stdout);
  run_execle(path, env);

  printf("\nЗапуск execv:\n");
  fflush(stdout);
  run_execv(path, args);

  printf("\nЗапуск execvp:\n");
  fflush(stdout);
  run_execvp(file, args);

  printf("\nЗапуск execve (с пользовательским окружением):\n");
  fflush(stdout);
  run_execve(path, args, env);

  printf("\nВсе дочерние процессы завершены.\n");

  return 0;
}
