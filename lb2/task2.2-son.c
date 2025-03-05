#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Son process is running with PID: %d\n", getpid());

    // Фиксируем состояние таблицы процессов
    system("ps -H f | head -n 7 > son_processes.txt");

    printf("Son process finished.\n");

    return 0;
}

