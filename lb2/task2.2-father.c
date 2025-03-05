#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Код для потомка
        execl("./son", "son", NULL);
        // Если execl вернул управление, значит произошла ошибка
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Код для родителя
        printf("Father process is running with PID: %d\n", getpid());
        printf("Child process created with PID: %d\n", pid);

        // Ждем завершения потомка
        wait(NULL);

        // Фиксируем состояние таблицы процессов
        system("ps -H f | head -n 6 > father_processes.txt");

        printf("Father process finished.\n");
    }

    return 0;
}