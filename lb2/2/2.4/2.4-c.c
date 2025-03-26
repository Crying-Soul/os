/**
 * 2.4-c.c
 * 
 * Функциональность:
 * 1. Демонстрация состояния зомби-процессов:
 *    - Создание потомка с последующим игнорированием родителем
 *    - Использование WNOHANG для неблокирующего ожидания
 *    - Визуализация процессов в состоянии "zombie"
 * 
 * 2. Техники мониторинга:
 *    - Специализированный вывод ps (STAT, SIZE)
 *    - Анализ результатов waitpid с разными флагами
 *    - Фиксация состояния системы после завершения потомка
 * 
 * 3. Особенности реализации:
 *    - Имитация "плохого" паттерна работы с процессами
 *    - Демонстрация последствий отсутствия wait()
 *    - Наглядное представление утечек ресурсов
 */
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