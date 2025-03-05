#!/bin/bash
cd 2.4
# Создаем файл для ситуации а
cat <<EOF > situation_a.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void print_processes(const char *message) {
    printf("\n%s\n", message);
    system("ps -l");
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Процесс-сын
        printf("Процесс-сын (PID: %d) запущен.\n", getpid());
        print_processes("Состояние процессов (сын):");
        sleep(2); // Имитация работы
        printf("Процесс-сын (PID: %d) завершен.\n", getpid());
        exit(0);
    } else if (pid > 0) {
        // Процесс-отец
        printf("Процесс-отец (PID: %d) ожидает завершения процесса-сына.\n", getpid());
        print_processes("Состояние процессов (отец):");
        wait(NULL); // Ожидание завершения процесса-сына
        printf("Процесс-отец (PID: %d) завершен.\n", getpid());
    } else {
        perror("Ошибка при вызове fork()");
        exit(1);
    }
    return 0;
}
EOF

# Создаем файл для ситуации б
cat <<EOF > situation_b.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void print_processes(const char *message) {
    printf("\n%s\n", message);
    system("ps -l");
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
EOF

# Создаем файл для ситуации в
cat <<EOF > situation_c.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void print_processes(const char *message) {
    printf("\n%s\n", message);
    system("ps -l");
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
        printf("Процесс-отец (PID: %d) не ожидает завершения процесса-сына.\n", getpid());
        sleep(4); // Имитация работы
        print_processes("Состояние процессов после завершения сына (зомби):");
        printf("Процесс-отец (PID: %d) завершен.\n", getpid());
    } else {
        perror("Ошибка при вызове fork()");
        exit(1);
    }
    return 0;
}
EOF

echo "Файлы situation_a.c, situation_b.c и situation_c.c созданы."