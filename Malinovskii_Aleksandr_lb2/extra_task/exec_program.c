#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    printf("Программа после exec PID: %d\n", getpid());
    
    // Формируем путь к файлу status
    char status_path[256];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", getpid());
    
    // Открываем файл
    FILE *status_file = fopen(status_path, "r");
    if (!status_file) {
        perror("Не удалось открыть файл status");
        exit(1);
    }
    
    printf("\nСодержимое /proc/%d/status в коде (сигналы):\n", getpid());
    
    // Читаем файл построчно и выводим информацию о сигналах
    char line[256];
    while (fgets(line, sizeof(line), status_file)) {
        if (strstr(line, "SigBlk") || strstr(line, "SigIgn") || strstr(line, "SigCgt")) {
            printf("%s", line);
        }
    }
    
    fclose(status_file);

    sleep(1);
    printf("Вывод  ps -o pid,sig,blocked,ignored,caught\n");
    char command[256];
    snprintf(command, sizeof(command), "ps -o pid,ppid,sig,blocked,ignored,caught -p %d", getpid());
    system(command);
    snprintf(command, sizeof(command), "ps -o pid,ppid,sig,blocked,ignored,caught -p %d", getppid());
    system(command);

    // Демонстрационная работа программы
    int  a = 0;
    for (size_t i = 0; i < 1000000000; i++) {
        a += 1;
    }
    
    return 0;
}