#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

// Функция для вывода информации о памяти процесса
void print_process_maps(pid_t pid) {
    char command[256];
    sprintf(command, "pmap -x %d", pid);  // Используем pmap для вывода информации о памяти
    printf("\n--- Memory usage for process %d (using pmap) ---\n", pid);
    system(command);  // Выполнение команды pmap
}

// Функция для имитации длительных вычислений (версия 1)
void long_computation(const char* process_name, int computations_amount) {
    for (int i = 0; i < computations_amount; i++) {
        printf("%s: PID = %d, PPID = %d, выполнение итерации %d\n", process_name, getpid(), getppid(), i);
    }
}

int main() {
    // Открываем файл для записи
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    // Перенаправляем стандартный вывод и вывод ошибок в файл
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    // Создаем переменную в родительском процессе
    int parent_var = 42;
    int *parent_var_ptr = &parent_var;

    printf("Родитель: Адрес переменной parent_var = %p, значение = %d\n", (void*)parent_var_ptr, *parent_var_ptr);

    print_process_maps(getpid());
    pid_t ret = fork();  // Создаем новый процесс

    if (ret < 0) {
        // Ошибка при создании процесса
        fprintf(stderr, "Ошибка при создании процесса\n");
        return 1;
    } else if (ret == 0) {
        // Код, выполняемый в процессе-потомке
        int *child_var_ptr = (int*)parent_var_ptr;  // Используем тот же адрес
        *child_var_ptr = 100;  // Изменяем значение по этому адресу

        printf("Потомок: Адрес переменной child_var = %p, значение = %d\n", (void*)child_var_ptr, *child_var_ptr);

        print_process_maps(getpid());
        long_computation("Потомок", 1000);
    } else {
        sleep(1);
        // Код, выполняемый в процессе-родителе
        printf("Родитель: После fork() адрес переменной parent_var = %p, значение = %d\n", (void*)parent_var_ptr, *parent_var_ptr);

        long_computation("Родитель", 1000);

        // Ожидание завершения потомка
        wait(NULL);
    }

    printf("Завершение программы (PID = %d)\n", getpid());

    // Закрываем файл
    close(fd);

    return 0;
}