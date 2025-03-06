#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// Имитация длительных вычислений (используется только в этом файле)
static void long_computation(const char* process_name, int sleep_time, int* var) {
    for (int i = 0; i < 5; i++) {
        printf("%s: PID = %d, PPID = %d, var = %d, адрес = %p, итерация %d\n", 
               process_name, getpid(), getppid(), *var, (void*)var, i);
        fflush(stdout);
        sleep(sleep_time);
    }
}

int main(void) {
    int shared_var = 100; // Локальная переменная, изменения которой в дочернем процессе не затрагивают родителя

    printf("Начало программы. Родитель: PID = %d, var = %d, адрес = %p\n", 
           getpid(), shared_var, (void*)&shared_var);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка при вызове fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс: PID = %d, PPID = %d, var = %d, адрес = %p (до изменения)\n",
               getpid(), getppid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        shared_var += 50;  // Изменяем копию переменной в дочернем процессе
        printf("Дочерний процесс: PID = %d, изменённое значение var = %d, адрес = %p\n", 
               getpid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        long_computation("Дочерний процесс", 3, &shared_var);

        printf("Дочерний процесс: PID = %d, завершение работы\n", getpid());
        fflush(stdout);
        _exit(EXIT_SUCCESS); // Используем _exit для корректного завершения дочернего процесса
    } else {
        // Родительский процесс
        sleep(1);  // Небольшая задержка для наглядности вывода

        printf("Родитель: PID = %d, var = %d, адрес = %p (после fork)\n", 
               getpid(), shared_var, (void*)&shared_var);
        fflush(stdout);

        long_computation("Родитель", 2, &shared_var);

        if (waitpid(pid, NULL, 0) < 0) {
            perror("Ошибка при ожидании завершения дочернего процесса");
            exit(EXIT_FAILURE);
        }

        printf("Родитель: PID = %d, завершение работы\n", getpid());
        fflush(stdout);
    }

    printf("Завершение программы (PID = %d)\n", getpid());
    fflush(stdout);
    return EXIT_SUCCESS;
}
