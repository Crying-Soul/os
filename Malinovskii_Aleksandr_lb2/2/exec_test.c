#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>

void run_execl() {
    printf("\n=== Запуск процесса с execl() ===\n");
    execl("/bin/ls", "ls", "-l", NULL);
    perror("execl failed"); // Если execl вернул управление, значит произошла ошибка
}

void run_execle() {
    printf("\n=== Запуск процесса с execle() ===\n");
    char *env[] = {"MY_VAR=Hello", NULL};
    execle("/bin/bash", "bash", "-c", "echo $MY_VAR", NULL, env);
    perror("execle failed");
}

void run_execlp() {
    printf("\n=== Запуск процесса с execlp() ===\n");
    execlp("ls", "ls", "-a", NULL);
    perror("execlp failed");
}

void run_execv() {
    printf("\n=== Запуск процесса с execv() ===\n");
    char *args[] = {"ls", "-l", "-a", NULL};
    execv("/bin/ls", args);
    perror("execv failed");
}

void run_execvp() {
    printf("\n=== Запуск процесса с execvp() ===\n");
    char *args[] = {"ls", "-l", "-a", NULL};
    execvp("ls", args);
    perror("execvp failed");
}

void run_execvpe() {
    printf("\n=== Запуск процесса с execvpe() ===\n");
    char *args[] = {"bash", "-c", "echo $MY_VAR", NULL};
    char *env[] = {"MY_VAR=Hello", NULL};
    execvpe("bash", args, env);
    perror("execvpe failed");
}

void run_custom_env() {
    printf("\n=== Запуск процесса с измененным окружением ===\n");
    char *env[] = {"MY_VAR=Hello", "PATH=/usr/local/bin:/usr/bin:/bin", NULL};
    execle("/bin/bash", "bash", "-c", "echo $MY_VAR; echo $PATH", NULL, env);
    perror("execle failed");
}

void run_custom_args() {
    printf("\n=== Запуск процесса с массивом параметров ===\n");
    char *args[] = {"ls", "-l", "-a", "-h", NULL};
    execvp("ls", args);
    perror("execvp failed");
}

int main() {
    // Примеры использования функций семейства exec()
    //run_execl();
    run_execle();
    run_execlp();
    run_execv();
    run_execvp();
    run_execvpe();
    run_custom_env();
    run_custom_args();

    return 0;
}