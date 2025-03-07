#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void print_processes(const char *message) {
  printf("\n%s\n", message);
  fflush(stdout);                              // Сброс буфера stdout
  system("ps -o pid,ppid,pgid,sid,comm -H f"); // Подробный вывод процессов
}

int main() {
  printf("Запуск программы. Основной процесс (PID: %d, PPID: %d).\n", getpid(),
         getppid());
  fflush(stdout);

  pid_t pid = fork(); // Создаем процесс-сына

  if (pid == 0) {
    // Код, выполняемый процессом-сыном
    printf("Процесс-сын (PID: %d, PPID: %d) запущен.\n", getpid(), getppid());
    fflush(stdout);

    // Имитация работы процесса-сына
    sleep(4);

    // Вывод состояния процессов после завершения отца
    printf("Процесс-сын (PID: %d, PPID: %d) после завершения отца.\n", getpid(),
           getppid());
    fflush(stdout);

    // Объяснение изменения PPID
    if (getppid() == 1) {
      printf(
          "Теперь родитель процесса-сына — процесс с PID 1 (init/systemd).\n");
      printf("Это корневой процесс системы, который управляет всеми процессами "
             "и завершает осиротевшие процессы.\n");
    } else {
      printf("Родитель процесса-сына: %d.\n", getppid());
    }
    fflush(stdout);

    // Дополнительная задержка для демонстрации изменения PPID
    sleep(2);

    printf("Процесс-сын (PID: %d, PPID: %d) завершен.\n", getpid(), getppid());
    fflush(stdout);
    exit(0);
  } else if (pid > 0) {
    // Код, выполняемый процессом-отцом
    printf("Процесс-отец (PID: %d) создал процесс-сына (PID: %d).\n", getpid(),
           pid);
    fflush(stdout);

    // Имитация работы процесса-отца
    sleep(1);

    // Вывод состояния процессов перед завершением отца
    print_processes("Состояние процессов перед завершением отца:");

    printf("Процесс-отец (PID: %d) завершается, не ожидая завершения "
           "процесса-сына.\n",
           getpid());
    fflush(stdout);
    exit(0);
  } else {
    // Обработка ошибки fork()
    perror("Ошибка при вызове fork()");
    fflush(stderr);
    exit(1);
  }

  return 0;
}