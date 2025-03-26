/**
 * 3.2-father.c
 *
 * Функциональность:
 * 1. Управление группой из 5 дочерних процессов с разными стратегиями завершения
 * 2. Ключевые операции:
 *    - Параллельный запуск процессов через fork()+execl()
 *    - Использование различных флагов для waitpid():
 *      * WNOHANG - неблокирующее ожидание
 *      * WUNTRACED - отслеживание остановленных процессов
 *    - Анализ статусов завершения (WIFEXITED)
 *    - Логирование иерархии процессов (ps -H f)
 *
 * 3. Особенности:
 *    - Демонстрация разных способов завершения процессов
 *    - Сравнение работы return и exit
 *    - Обработка различных кодов возврата
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main() {
  int i, ppid, pid[6], status[5], result[5];
  char *son[] = {"3.2-son1", "3.2-son2", "3.2-son3", "3.2-son4", "3.2-son5"};
  char *sonPath[] = {"./src/3/3.2/3.2-son1", "./src/3/3.2/3.2-son2",
                     "./src/3/3.2/3.2-son3", "./src/3/3.2/3.2-son4",
                     "./src/3/3.2/3.2-son5"};
  int option[] = {WNOHANG, WUNTRACED, WNOHANG, WNOHANG, WNOHANG};

  pid[5] = getpid();
  ppid = getppid();
  printf("Father parameters was: pid=%i ppid=%i\n", pid[5], ppid);
  for (i = 0; i < 5; i++) {
    if ((pid[i] = fork()) == 0) {
      execl(sonPath[i], son[i], NULL);
    }
  }

  system("ps -H f > file.txt");
  for (i = 0; i < 5; i++) {
    result[i] = waitpid(pid[i], &status[i], option[i]);
    printf("%d) Child with pid = %d is finished with status %d\n", (1 + i),
           result[i], status[i]);
  }

  for (i = 0; i < 5; i++)
    if (WIFEXITED(status[i]) == 0) {
      printf("Proccess pid = %d was failed.\n", pid[i]);
    } else {
      printf("Proccess pid = %d was success.\n", pid[i]);
    }
  return 0;
}