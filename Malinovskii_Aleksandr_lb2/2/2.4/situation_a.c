#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Вывод процессов до создания процесса-сына
    printf("Процессы до создания процесса-сына:\n");
    system("ps -H f");  // Вывод списка процессов

    // Создаем процесс-сын
    pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        fprintf(stderr, "Ошибка при вызове fork()\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Код, выполняемый процессом-сыном
        printf("Процесс-сын: PID = %d\n", getpid());
        sleep(2);  // Имитация работы
        printf("Процесс-сын завершен\n");
        exit(EXIT_SUCCESS);
    } else {
        // Код, выполняемый процессом-отцом
        printf("Процесс-отец: PID = %d, PID сына = %d\n", getpid(), pid);

        // Вывод процессов после создания процесса-сына, но до его завершения
        printf("Процессы после создания процесса-сына, но до его завершения:\n");
        system("ps -H f");

        int status;
        wait(&status);  // Ожидание завершения процесса-сына

        // Вывод процессов после завершения процесса-сына
        printf("Процессы после завершения процесса-сына:\n");
        system("ps -H f");
        
        if (WIFEXITED(status)) {
            printf("Процесс-сын завершился с кодом: %d\n", WEXITSTATUS(status));
        } else {
            printf("Процесс-сын завершился некорректно\n");
        }
        printf("Процесс-отец завершен\n");
    }

    return 0;
}