/**
 * 5.1-prog3.c
 * 
 * Функциональность:
 * - Тестовый процесс для анализа планировщика
 * - Бесконечный цикл с выводом:
 *   * PID процесса
 *   * Текущее nice-значение (getpriority)
 *   * Политика планирования (sched_getscheduler)
 * 
 * Особенности:
 * - Принудительный сброс буфера вывода (fflush)
 * - Минимальная нагрузка (комментирован CPU-intensive код)
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sched.h>

int main() {
    int count = 0;
    while(1) {
        // for(int i = 0; i < 10000000; i++) count++;
        // printf("3: PID=%d, nice=%d, policy=%d\n",
        //        getpid(),
        //        getpriority(PRIO_PROCESS, 0),
        //        sched_getscheduler(0));
        // fflush(stdout);
    }
    return 0;
}
