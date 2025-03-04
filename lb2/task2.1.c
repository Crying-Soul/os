#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// Функция для вывода информации о памяти процесса
void print_process_maps(pid_t pid) {
    char command[256];
    sprintf(command, "pmap -x %d", pid);  // Используем pmap для вывода информации о памяти
    printf("\n--- Memory usage for process %d (using pmap) ---\n", pid);
    system(command);  // Выполнение команды pmap
}

// Функция для имитации длительных вычислений (версия 1)
void long_computation(const char* process_name, int sleep_time) {
    // printf("%s: PID = %d, PPID = %d\n", process_name, getpid(), getppid());

    for (int i = 0; i < 5; i++) {
        printf("%s: PID = %d, PPID = %d, выполнение итерации %d\n", process_name, getpid(), getppid(), i);
        sleep(sleep_time);  // Имитация длительных вычислений
    }
}

int main() {
    print_process_maps(getpid());
    pid_t ret = fork();  // Создаем новый процесс

    if (ret < 0) {
        // Ошибка при создании процесса
        fprintf(stderr, "Ошибка при создании процесса\n");
        return 1;
    } else if (ret == 0) {
        print_process_maps(getpid());  // Выводим информацию о памяти текущего процесса
        // Код, выполняемый в процессе-потомке
        long_computation("Потомок",3);
    } else {
        sleep(1);
        // Код, выполняемый в процессе-родителе
        long_computation("Родитель",4);

        // Ожидание завершения потомка
        wait(NULL);
    }

    printf("Завершение программы (PID = %d)\n", getpid());
    return 0;
}
