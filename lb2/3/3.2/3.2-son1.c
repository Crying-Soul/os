/**
 * 3.2-son1.c
 *
 * Функциональность:
 * - Демонстрация нормального завершения через return 0
 * - Вывод PID/PPID для идентификации в системе
 *
 * Особенности:
 * - Корректное завершение с кодом 0 (успех)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);

  return 0; // son1: return 0
}