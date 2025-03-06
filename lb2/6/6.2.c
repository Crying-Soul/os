#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>

void test_priority_inheritance(void) {
    printf("=== Тест 6.2: Наследование приоритетов и политики планирования ===\n");
    fflush(stdout);

    if (nice(5) == -1 && errno != 0) {
        perror("nice");
    }
    int prio = getpriority(PRIO_PROCESS, 0);
    int policy = sched_getscheduler(0);
    printf("Родитель (PID %d): приоритет = %d, политика планирования = %d\n", getpid(), prio, policy);
    fflush(stdout);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        int child_prio = getpriority(PRIO_PROCESS, 0);
        int child_policy = sched_getscheduler(0);
        printf("Потомок (PID %d): приоритет = %d, политика планирования = %d\n", getpid(), child_prio, child_policy);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }
    
    printf("Завершён тест наследования приоритетов.\n\n");
    fflush(stdout);
}

int main() {
    test_priority_inheritance();
    return 0;
}
