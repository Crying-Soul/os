#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main() {
    pid_t pid = fork();  // Создаем новый процесс

    if (pid < 0) {
        // Ошибка при создании процесса
        fprintf(stderr, "Ошибка при создании процесса\n");
        return 1;
    } else if (pid == 0) {
        // Код, выполняемый в процессе-потомке
        printf("Потомок: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Единичное вычисление потомка");
    } else {
        // Код, выполняемый в процессе-родителе
        printf("Родитель: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Единичное вычисление родителя");
    }

    // Ожидание завершения потомка (если это родитель)
    if (pid > 0) {
        wait(NULL);
    }

    printf("Завершение программы (PID = %d)\n", getpid());
    return 0;
}