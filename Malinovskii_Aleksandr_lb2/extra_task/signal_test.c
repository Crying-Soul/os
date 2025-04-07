#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

// Обработчики сигналов
void handler1(int sig) {
    printf("Handler1: Получен сигнал %d\n", sig);
}

void handler2(int sig) {
    printf("Handler2: Получен сигнал %d\n", sig);
}

// Функция для вывода информации о сигналах из /proc/[pid]/status
void print_signal_status(pid_t pid) {
    char status_path[256];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
    
    FILE *status_file = fopen(status_path, "r");
    if (!status_file) {
        perror("Не удалось открыть файл status");
        return;
    }
    
    printf("\nСодержимое /proc/%d/status (сигналы):\n", pid);
    
    char line[256];
    while (fgets(line, sizeof(line), status_file)) {
        if (strstr(line, "SigBlk") || strstr(line, "SigIgn") || strstr(line, "SigCgt")) {
            printf("%s", line);
        }
    }
    
    fclose(status_file);
}

// Функция для тестирования fork
void test_fork() {
    printf("\n=== Тестирование fork() ===\n");
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний процесс PID: %d\n", getpid());
        print_signal_status(getpid());
        sleep(1);
        printf("Вывод  ps -o pid,sig,blocked,ignored,caught\n");
        char command[256];
        snprintf(command, sizeof(command), "ps -o pid,ppid,sig,blocked,ignored,caught -p %d", getpid());
        system(command);
        snprintf(command, sizeof(command), "ps -o pid,ppid,sig,blocked,ignored,caught -p %d", getppid());
        system(command);
        exit(0);
    } else if (pid > 0) {
        printf("Родительский процесс PID: %d\n", getpid());
        print_signal_status(getpid());
        wait(NULL);
    } else {
        perror("fork");
        exit(1);
    }
}

// Функция для тестирования exec
void test_exec() {
    printf("\n=== Тестирование exec() ===\n");
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний процесс перед exec PID: %d\n", getpid());
        
        char *argv[] = {NULL};
        execvp("./exec_program", argv);
        
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        printf("Родительский процесс PID: %d\n", getpid());
        print_signal_status(getpid());
        wait(NULL);
    } else {
        perror("fork");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <fork|exec>\n", argv[0]);
        exit(1);
    }

    // Установка обработчиков
    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);
    
    // Игнорирование сигналов
    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    // Блокировка сигналов
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    sigaddset(&block_set, SIGQUIT);
    sigprocmask(SIG_BLOCK, &block_set, NULL);

    if (strcmp(argv[1], "fork") == 0) {
        test_fork();
    } else if (strcmp(argv[1], "exec") == 0) {
        test_exec();
    } else {
        fprintf(stderr, "Неизвестный аргумент: %s\n", argv[1]);
        fprintf(stderr, "Использование: %s <fork|exec>\n", argv[0]);
        exit(1);
    }
    
    return 0;
}