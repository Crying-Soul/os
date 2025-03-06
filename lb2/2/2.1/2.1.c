#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <math.h>

// Имитация длительных вычислений
static void long_computation(const char *process_name, int *var)
{
    for (int i = 0; i < 100000000; i++)
    {
        if (i % 10000000 == 0)
        {
            printf("%-24s | PID = %-1d | PPID = %-1d | var = %-1d | адрес = %-14p | Итерация = %-10d\n",
                   process_name, getpid(), getppid(), *var, (void *)var, i);
            fflush(stdout);
        }
    }
}

int main(void)
{
    int shared_var = 100; // Локальная переменная, не разделяемая между процессами

    printf("%-24s | PID = %-1d | var = %-1d | адрес = %-14p\n",
           "Начало программы", getpid(), shared_var, (void *)&shared_var);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Ошибка при вызове fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Дочерний процесс
        printf("%-24s | PID = %-1d | PPID = %-1d | var = %-1d | адрес = %-14p | (до изменения)\n",
               "Дочерний процесс", getpid(), getppid(), shared_var, (void *)&shared_var);
        fflush(stdout);

        shared_var += 50; // Изменяем копию переменной в дочернем процессе
        printf("%-24s | PID = %-1d | PPID = %-1d | var = %-1d | адрес = %-14p | (после изменения)\n",
               "Дочерний процесс", getpid(), getppid(), shared_var, (void *)&shared_var);
        fflush(stdout);

        long_computation("Дочерний процесс", &shared_var);

        printf("%-24s | PID = %-1d | Завершение работы\n", "Дочерний процесс", getpid());
        fflush(stdout);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        // Родительский процесс
        printf("%-24s | PID = %-1d | PPID = %-1d | var = %-1d | адрес = %-14p | (после fork)\n",
               "Родитель", getpid(), getppid(), shared_var, (void *)&shared_var);
        fflush(stdout);

        long_computation("Родитель", &shared_var);

        if (waitpid(pid, NULL, 0) < 0)
        {
            perror("Ошибка при ожидании завершения дочернего процесса");
            exit(EXIT_FAILURE);
        }

        printf("%-24s | PID = %-1d | Завершение работы\n", "Родитель", getpid());
        fflush(stdout);
    }

    printf("%-21s | PID = %-1d | Завершение программы\n", "Конец", getpid());
    fflush(stdout);
    return EXIT_SUCCESS;
}
