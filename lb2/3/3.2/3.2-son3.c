/**
 * 3.2-son3.c
 *
 * Функциональность:
 * - Демонстрация завершения с отрицательным кодом через exit(-1)
 * - Вывод PID/PPID
 *
 * Особенности:
 * - Возврат отрицательного кода (нестандартная ситуация)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);
  exit(-1); // son3: exit(-1)
}