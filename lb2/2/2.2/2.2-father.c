/**
 * 2.2-father.c
 * 
 * Функциональность:
 * 1. Создание и управление дочерним процессом:
 *    - Разветвление выполнения через fork()
 *    - Запуск внешней программы в потомке через execl()
 *    - Ожидание завершения потомка через wait()
 * 
 * 2. Системный мониторинг:
 *    - Логирование иерархии процессов (ps -H f)
 *    - Фиксация PID созданных процессов
 *    - Детекция ошибок fork() и execl()
 * 
 * 3. Особенности реализации:
 *    - Полный контроль жизненного цикла потомка
 *    - Двойная обработка ошибок (fork + execl)
 *    - Сохранение снимка состояния процессов
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();

  if (pid < 0) {
    // Ошибка при создании процесса
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // Код для потомка
    execl("./src/2/2.2/2.2-son", "2.2-son", NULL);
    // Если execl вернул управление, значит произошла ошибка
    perror("execl");
    exit(EXIT_FAILURE);
  } else {
    // Код для родителя
    printf("Father process is running with PID: %d\n", getpid());
    printf("Child process created with PID: %d\n", pid);

    // Ждем завершения потомка
    wait(NULL);

    // Фиксируем состояние таблицы процессов
    system("ps -H f | tee father_processes.log");

    printf("Father process finished.\n");
  }

  return 0;
}