/**
 * 3.2-son5.c
 *
 * Функциональность:
 * - Демонстрация завершения с большим отрицательным кодом через exit(-10)
 * - Вывод PID/PPID
 *
 * Особенности:
 * - Возврат значительного отрицательного кода
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);

  exit(-10);
}