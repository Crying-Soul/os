#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    struct sched_param param;
    struct timespec quantum;
    pid_t pid, ppid, child_pid;

    pid = getpid();
    ppid = getppid();
    printf("FATHER: pid=%d, ppid=%d\n", pid, ppid);

    // Устанавливаем политику планирования SCHED_RR с приоритетом 50
    param.sched_priority = 50;
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    // Получаем значение кванта времени для SCHED_RR
    if (sched_rr_get_interval(0, &quantum) == 0)
        printf("FATHER: Round-robin quantum: %.9f seconds\n",
               quantum.tv_sec + quantum.tv_nsec / 1e9);
    else {
        perror("sched_rr_get_interval");
        exit(EXIT_FAILURE);
    }

    // Создаем дочерний процесс
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {  // Дочерний процесс
        // Получаем значение кванта времени для SCHED_RR в дочернем процессе
        if (sched_rr_get_interval(0, &quantum) == 0)
            printf("SON: Round-robin quantum: %.9f seconds\n",
                   quantum.tv_sec + quantum.tv_nsec / 1e9);
        else {
            perror("sched_rr_get_interval in child");
            exit(EXIT_FAILURE);
        }
    } else {  // Родительский процесс
        int status;
        // Ожидаем завершения дочернего процесса
        if (waitpid(child_pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        
    }

    return 0;
}