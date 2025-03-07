#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <math.h>

// Имитация длительных вычислений
static void compute(const char* process_name, int* var) {
    for (int i = 0; i < 1000000; i++) {  // Уменьшено количество итераций
        if (i % 100000 == 0) {  // Вывод каждые 100000 итераций
            printf("[%s] PID=%d, PPID=%d, var=%d, addr=%p, iter=%d\n", 
                    
                   process_name, getpid(), getppid(), *var, (void*)var, i);
            fflush(stdout);
        }
        *var+=1;
    }
}

int main(void) {
    int shared_var = 4; // Локальная переменная, не разделяемая между процессами

    printf("[Start] PID=%d, var=%d, addr=%p\n", getpid(), shared_var, (void*)&shared_var);
    fflush(stdout);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("[Child] PID=%d, PPID=%d, var=%d, addr=%p (before change)\n",
               getpid(), getppid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        shared_var += 23;  // Изменяем копию переменной в дочернем процессе
        printf("[Child] PID=%d, PPID=%d, var=%d, addr=%p (after change)\n", 
               getpid(), getppid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        compute("Child", &shared_var);

        printf("[Child] PID=%d, exiting\n", getpid());
        fflush(stdout);
        _exit(EXIT_SUCCESS);
    } else {
        // Родительский процесс
        printf("[Parent] PID=%d, PPID=%d, var=%d, addr=%p (after fork)\n", 
               getpid(), getppid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        compute("Parent", &shared_var);

        if (waitpid(pid, NULL, 0) < 0) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }

        printf("[Parent] PID=%d, exiting\n", getpid());
        fflush(stdout);
    }

    printf("[End] PID=%d, program finished\n", getpid());
    fflush(stdout);
    return EXIT_SUCCESS;
}