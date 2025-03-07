#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid;

  // Вывод процессов до создания процесса-сына
  printf("Процессы до создания процесса-сына:\n");
  fflush(stdout);    // Сброс буфера stdout
  system("ps -H f"); // Вывод списка процессов

  // Создаем процесс-сын
  pid = fork();

  if (pid < 0) {
    // Ошибка при создании процесса
    fprintf(stderr, "Ошибка при вызове fork()\n");
    fflush(stderr); // Сброс буфера stderr
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // Код, выполняемый процессом-сыном
    printf("Процесс-сын: PID = %d\n", getpid());
    fflush(stdout); // Сброс буфера stdout
    sleep(2);       // Имитация работы
    printf("Процесс-сын завершен\n");
    fflush(stdout); // Сброс буфера stdout
    exit(EXIT_SUCCESS);
  } else {
    // Код, выполняемый процессом-отцом
    printf("Процесс-отец: PID = %d, PID сына = %d\n", getpid(), pid);
    fflush(stdout); // Сброс буфера stdout

    // Вывод процессов после создания процесса-сына, но до его завершения
    printf("Процессы после создания процесса-сына, но до его завершения:\n");
    fflush(stdout); // Сброс буфера stdout
    system("ps -H f");

    int status;
    wait(&status); // Ожидание завершения процесса-сына

    // Вывод процессов после завершения процесса-сына
    printf("Процессы после завершения процесса-сына:\n");
    fflush(stdout); // Сброс буфера stdout
    system("ps -H f");

    if (WIFEXITED(status)) {
      printf("Процесс-сын завершился с кодом: %d\n", WEXITSTATUS(status));
    } else {
      printf("Процесс-сын завершился некорректно\n");
    }
    fflush(stdout); // Сброс буфера stdout
    printf("Процесс-отец завершен\n");
    fflush(stdout); // Сброс буфера stdout
  }

  return 0;
}