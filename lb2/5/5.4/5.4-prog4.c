/**
* 5.4-prog1.c / 5.4-prog2.c / 5.4-prog3.c / 5.4-prog3.c
*
* Функциональность:
* 1. Демонстрация планирования SCHED_RR (Round Robin):
*    - Установка политики SCHED_RR с заданным приоритетом
*    - Выполнение CPU-intensive нагрузки (цикл с инкрементом)
*    - Периодический вывод информации о процессе
*
* 2. Ключевые отличия от FIFO-версии:
*    - Использование SCHED_RR вместо SCHED_FIFO
*    - Процессы с одинаковым приоритетом разделяют время CPU
*    - Добавлен бесконечный цикл для наглядности планирования
*
* 3. Особенности реализации:
*    - Требует прав root для установки SCHED_RR
*    - Привязка к ядру CPU через taskset в скриптах
*    - Валидация приоритета в диапазоне 1-99
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <limits.h>

void print_process_info(const char* process_name) {

    // Получаем текущую политику планирования
    int policy = sched_getscheduler(0);
    const char* policy_name;
    switch (policy) {
        case SCHED_FIFO:
            policy_name = "SCHED_FIFO";
            break;
        case SCHED_RR:
            policy_name = "SCHED_RR";
            break;
        case SCHED_OTHER:
            policy_name = "SCHED_OTHER";
            break;
        default:
            policy_name = "Неизвестно";
    }
    
    // Получаем текущий приоритет
    struct sched_param param;
    if (sched_getparam(0, &param) == -1) {
        perror("sched_getparam failed");
        exit(EXIT_FAILURE);
    }
    
    printf("| %-s | PID: %-d | %-s | Приоритет: %d\n", process_name, getpid(), policy_name, param.sched_priority);
   
}

void set_rr_scheduling(int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    int priority;
    int min_priority = sched_get_priority_min(SCHED_FIFO);
    int max_priority = sched_get_priority_max(SCHED_FIFO);

    if (argc > 1) {
        char *endptr;
        errno = 0;
        long p = strtol(argv[1], &endptr, 10);
        if (errno != 0 || *endptr != '\0' || p < min_priority || p > max_priority) {
            fprintf(stderr, "Недопустимый приоритет. Используйте значение от %d до %d.\n", min_priority, max_priority);
            exit(EXIT_FAILURE);
        }
        priority = (int)p;
    } else {
        priority = (min_priority + max_priority) / 2;
    }

    set_rr_scheduling(priority);

    while(1){
        volatile unsigned long dummy = 0;
        for (unsigned long j = 0; j < 1000000000UL; j++){
            dummy++;
        }
        print_process_info("process4");
    }
    return 0;
}