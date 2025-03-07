#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void print_processes(const char *message) {
  printf("\n%s\n", message);
  fflush(stdout); // Сброс буфера stdout
  system("ps -o pid,ppid,stat,comm,size");
}

int main() {
  pid_t pid = fork();
  if (pid == 0) {
    // Процесс-сын
    printf("Процесс-сын (PID: %d) запущен.\n", getpid());
    fflush(stdout); // Сброс буфера stdout
    sleep(2);       // Имитация работы
    printf("Процесс-сын (PID: %d) завершен.\n", getpid());
    fflush(stdout); // Сброс буфера stdout
    exit(0);
  } else if (pid > 0) {
    // Процесс-отец
    printf("Процесс-отец (PID: %d) не ожидает завершения процесса-сына.\n",
           getpid());
    fflush(stdout); // Сброс буфера stdout

    // Используем waitpid с флагом WNOHANG
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == 0) {
      printf(
          "Процесс-сын еще не завершился, процесс-отец продолжает работу.\n");
    } else if (result > 0) {
      if (WIFEXITED(status)) {
        printf("Процесс-сын завершился с кодом: %d\n", WEXITSTATUS(status));
      } else {
        printf("Процесс-сын завершился некорректно.\n");
      }
    } else {
      perror("Ошибка при вызове waitpid");
    }
    fflush(stdout);

    sleep(4); // Имитация работы
    print_processes("Состояние процессов после завершения сына (зомби):");
    printf("Процесс-отец (PID: %d) завершен.\n", getpid());
    fflush(stdout); // Сброс буфера stdout
  } else {
    perror("Ошибка при вызове fork()");
    fflush(stderr); // Сброс буфера stderr
    exit(1);
  }
  return 0;
}