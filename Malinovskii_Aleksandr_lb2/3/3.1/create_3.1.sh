#!/bin/bash

# Создаем директорию 3.1, если она не существует



# Создаем файл father.c
cat <<EOF > father.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t son1, son2, son3;

    // Создаем три дочерних процесса
    son1 = fork();
    if (son1 == 0) {
        execl("./son1", "son1", NULL);
    }

    son2 = fork();
    if (son2 == 0) {
        execl("./son2", "son2", NULL);
    }

    son3 = fork();
    if (son3 == 0) {
        execl("./son3", "son3", NULL);
    }

    // Даем время дочерним процессам запуститься
    sleep(2);

    // Отправляем сигналы дочерним процессам
    kill(son1, SIGUSR1);
    kill(son2, SIGUSR1);
    kill(son3, SIGUSR1);

    // Ждем завершения дочерних процессов
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}
EOF

# Создаем файл son1.c
cat <<EOF > son1.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main() {
    printf("son1: Waiting for signal...\n");
    pause();  // Ожидание сигнала
    printf("son1: Received signal, exiting...\n");
    return 0;
}
EOF

# Создаем файл son2.c
cat <<EOF > son2.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main() {
    signal(SIGUSR1, SIG_IGN);  // Игнорирование сигнала
    printf("son2: Waiting for signal...\n");
    pause();  // Ожидание сигнала
    printf("son2: Received signal, but ignoring it...\n");
    return 0;
}
EOF

# Создаем файл son3.c
cat <<EOF > son3.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handle_signal(int sig) {
    printf("son3: Received signal %d, handling it...\n", sig);
}

int main() {
    signal(SIGUSR1, handle_signal);  // Установка обработчика сигнала
    printf("son3: Waiting for signal...\n");
    pause();  // Ожидание сигнала
    printf("son3: Signal handled, exiting...\n");
    return 0;
}
EOF

echo "Файлы созданы в директории 3.1!"