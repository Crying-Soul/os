/**
 * 3.1-son2.c
 * 
 * Функциональность:
 * 1. Демонстрация игнорирования сигналов:
 *    - Явное игнорирование SIGINT через SIG_IGN
 *    - Автоматическое завершение через 5 секунд
 * 
 * 2. Особенности:
 *    - Показывает устойчивость к стандартным сигналам
 *    - Контролируемое завершение по таймеру
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  printf("Son2: Started (PID: %d)\n", getpid());
  signal(SIGINT, SIG_IGN); // Игнорируем SIGINT
  sleep(5);                // Работаем 5 секунд
  printf("Son2: Exiting after 5 seconds.\n");
  exit(0);
}