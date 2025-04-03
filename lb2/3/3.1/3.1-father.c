/**
 * 3.1-father.c
 * 
 * Функциональность:
 * 1. Управление группой дочерних процессов с отправкой сигналов:
 *    - Последовательный запуск трех типов процессов
 *    - Контролируемая отправка SIGINT каждому процессу
 *    - Мониторинг состояния через ps с фильтрацией
 * 
 * 2. Ключевые операции:
 *    - Создание процессов через fork()+execl()
 *    - Принудительное завершение через kill(SIGINT)
 *    - Ожидание завершения через waitpid()
 *    - Логирование состояния в processes_after.txt
 * 
 * 3. Особенности реализации:
 *    - Унифицированная функция spawn_and_signal()
 *    - Очистка лог-файла перед запуском
 *    - Задержки для синхронизации процессов
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Функция для вывода информации о процессах (SID, PID, PPID, STAT, COMM)
void print_processes() {
  system("ps -s | grep -v -E \"(ps|tee|sh -c)\" | tee processes_after.txt");
}

// Функция для создания процесса, отправки SIGINT и ожидания завершения
void spawn_and_signal(const char *path, const char *proc_name) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("Ошибка fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    execl(path, proc_name, (char *)NULL);
    perror("Ошибка execl");
    exit(EXIT_FAILURE);
  }
  sleep(1);
  printf("Father: Отправляем SIGINT процессу %s (PID: %d)\n", proc_name, pid);
  kill(pid, SIGINT);
  print_processes();
  waitpid(pid, NULL, 0);
}

int main() {
  // Очищаем файл перед записью
  system("> processes_after.txt");

  spawn_and_signal("./src/3/3.1/3.1-son1", "src/3/3.1/3.1-son1");
  spawn_and_signal("./src/3/3.1/3.1-son2", "src/3/3.1/3.1-son2");
  spawn_and_signal("./src/3/3.1/3.1-son3", "src/3/3.1/3.1-son3");

  printf("Father: Все дочерние процессы завершены.\n");
  return 0;
}
