/**
 * 3.2-son2.c
 *
 * Функциональность:
 * - Демонстрация завершения с ошибкой через exit(1)
 * - Вывод PID/PPID для идентификации
 *
 * Особенности:
 * - Возврат ненулевого кода (1 - стандартная ошибка)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);
  exit(1); // son2: exit(1)
}