#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void normal_completion() {
    pid_t pid = fork();
    if (pid == 0) {
        // Процесс-сын
        printf("Процесс-сын (PID: %d) запущен.\n", getpid());
        sleep(2); // Имитация работы
        printf("Процесс-сын (PID: %d) завершен.\n", getpid());
        exit(0);
    } else if (pid > 0) {
        // Процесс-отец
        printf("Процесс-отец (PID: %d) ожидает завершения процесса-сына.\n", getpid());
        wait(NULL); // Ожидание завершения процесса-сына
        printf("Процесс-отец (PID: %d) завершен.\n", getpid());
    } else {
        perror("fork");
        exit(1);
    }
}

void parent_change() {
    pid_t pid = fork();
    if (pid == 0) {
        // Процесс-сын
        printf("Процесс-сын (PID: %d) запущен.\n", getpid());
        sleep(2); // Имитация работы
        printf("Процесс-сын (PID: %d) завершен.\n", getpid());
        exit(0);
    } else if (pid > 0) {
        // Процесс-отец
        printf("Процесс-отец (PID: %d) завершается, не дожидаясь завершения процесса-сына.\n", getpid());
        exit(0); // Завершаем процесс-отец
    } else {
        perror("fork");
        exit(1);
    }

    // После завершения процесса-отца подождем некоторое время,
    // чтобы родительский процесс у процесса-сына сменился.
    sleep(2);
    printf("Информация о процессах после завершения процесса-отца:\n");
    system("ps -o pid,ppid,state,comm"); // Вывод информации о процессах
}

void zombie_process() {
    pid_t pid = fork();
    if (pid == 0) {
        // Процесс-сын
        printf("Процесс-сын (PID: %d) запущен.\n", getpid());
        sleep(2); // Имитация работы
        printf("Процесс-сын (PID: %d) завершен.\n", getpid());
        exit(0);
    } else if (pid > 0) {
        // Процесс-отец
        printf("Процесс-отец (PID: %d) не ожидает завершения процесса-сына.\n", getpid());
        sleep(4); // Задержка, чтобы процесс-сын успел завершиться
        printf("Процесс-отец (PID: %d) завершен.\n", getpid());
        printf("Информация о процессах после завершения процесса-отца:\n");
        system("ps -o pid,ppid,state,comm"); // Вывод информации о процессах
    } else {
        perror("fork");
        exit(1);
    }
}

int main() {
    printf("Ситуация 1: Нормальное завершение процесса\n");
    normal_completion();

    printf("\nСитуация 2: Смена родителя\n");
    parent_change();
    
    printf("\nСитуация 3: Процесс-зомби\n");
    zombie_process();

    return 0;
}
