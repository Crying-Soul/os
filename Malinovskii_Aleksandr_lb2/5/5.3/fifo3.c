#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <limits.h>

void print_process_info(const char* process_name) {
    printf("Процесс: %s\n", process_name);
    printf("PID: %d\n", getpid());

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
    printf("Политика планирования: %s\n", policy_name);

    // Получаем текущий приоритет
    struct sched_param param;
    if (sched_getparam(0, &param) == -1) {
        perror("sched_getparam failed");
        exit(EXIT_FAILURE);
    }
    printf("Приоритет: %d\n", param.sched_priority);
    printf("========================\n");
}

void set_fifo_scheduling(int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    int priority;
    int min_priority = sched_get_priority_min(SCHED_FIFO);
    int max_priority = sched_get_priority_max(SCHED_FIFO);

    // Если передан аргумент, пытаемся его преобразовать в число
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
        // По умолчанию устанавливаем приоритет на уровне середины диапазона
        priority = (min_priority + max_priority) / 2;
    }

    set_fifo_scheduling(priority);

    // Имитация нагрузки и вывод информации о процессе
    for (int i = 0; i < 5; i++){
        volatile unsigned long dummy = 0;
        for (unsigned long j = 0; j < 1000000000UL; j++){
            dummy++;
        }
        print_process_info("process3");
    }
    return 0;
}