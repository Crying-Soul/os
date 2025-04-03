/**
 * sched_policy_info.c
 *
 * Функциональность:
 * 1. Анализ доступных политик планирования в Linux:
 *    - Определение минимальных и максимальных приоритетов
 *    - Поддержка трех основных политик: SCHED_FIFO, SCHED_RR, SCHED_OTHER
 *
 * 2. Ключевые операции:
 *    - Получение граничных значений приоритетов (sched_get_priority_min/max)
 *    - Форматированный вывод информации в виде таблицы
 *    - Обработка ошибок системных вызовов
 *
 */
#include <sched.h>
#include <stdio.h>
#include <stdlib.h> // для exit()

int main() {
  int policies[] = {SCHED_FIFO, SCHED_RR, SCHED_OTHER};
  const char *names[] = {"SCHED_FIFO", "SCHED_RR", "SCHED_OTHER"};

  int num_policies = sizeof(policies) / sizeof(policies[0]);

  printf("Policy\t\tMin Priority\tMax Priority\n");
  printf("---------------------------------------------\n");

  for (int i = 0; i < num_policies; i++) {
    int max = sched_get_priority_max(policies[i]);
    int min = sched_get_priority_min(policies[i]);

    if (max == -1 || min == -1) {
      perror("sched_get_priority_max/min");
      exit(EXIT_FAILURE);
    }

    printf("%-12s\t%5d\t\t%5d\n", names[i], min, max);
  }

  return 0;
}