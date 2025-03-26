/**
 * 3.2-son4.c
 *
 * Функциональность:
 * - Демонстрация завершения через return с положительным кодом
 * - Вывод PID/PPID
 *
 * Особенности:
 * - Возврат ненулевого положительного кода (5)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);

  return 5; // son4: return 5
}