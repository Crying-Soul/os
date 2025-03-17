#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

void print_process_info(const char* process_name) {
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

    struct sched_param param;
    if (sched_getparam(0, &param) == -1) {
        perror("sched_getparam failed");
        exit(EXIT_FAILURE);
    }

    printf("| %-s | PID: %-d | Политика: %-s | Приоритет: %d\n", process_name, getpid(), policy_name, param.sched_priority);
}

void set_other_scheduling() {
    struct sched_param param = {0};  
    if (sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    set_other_scheduling();  // Устанавливаем политику SCHED_OTHER

    while(1) {
        volatile unsigned long dummy = 0;
        for (unsigned long j = 0; j < 1000000000UL; j++) {
            dummy++;
        }
        print_process_info("process1");
    }

    return 0;
}
