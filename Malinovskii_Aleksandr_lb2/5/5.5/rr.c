#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>

static int count = 0;

void print_scheduling_info() {
    int policy = sched_getscheduler(0);
    const char *policy_name;
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
            policy_name = "Unknown";
            break;
    }

    struct sched_param param;
    sched_getparam(0, &param);
    int priority = param.sched_priority;

    printf("Program 2: PID %d: Computation %d, Scheduling Policy: %s, Priority: %d\n", getpid(),count++, policy_name, priority);
    fflush(stdout);
}

void perform_computation() {
    int counter = 0;
    for (int i = 0; i < 1000000000; i++) counter++; // Имитация вычислений
    print_scheduling_info();
}

int main() {
    struct sched_param param = {.sched_priority = 99}; // Одинаковый приоритет

    // Устанавливаем политику планирования RR
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        perror("sched_setscheduler");
        return 1;
    }

    // Выводим текущую политику планирования
    print_scheduling_info();

    while (1) {
        perform_computation();
        fflush(stdout);
        sched_yield();  // Передача управления другим процессам
    }

    return 0;
}