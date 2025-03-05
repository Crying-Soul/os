#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void print_processes(const char *message) {
    printf("\n%s\n", message);
    system("ps -H f ");
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Процесс-сын
        printf("Процесс-сын (PID: %d) запущен.\n", getpid());
        sleep(2); // Имитация работы
        printf("Процесс-сын (PID: %d) завершен.\n", getpid());
        exit(0);
    } else if (pid > 0) {
        // Процесс-отец
        printf("Процесс-отец (PID: %d) завершается, не ожидая завершения процесса-сына.\n", getpid());
        print_processes("Состояние процессов перед завершением отца:");
        exit(0);
    } else {
        perror("Ошибка при вызове fork()");
        exit(1);
    }
    return 0;
}
