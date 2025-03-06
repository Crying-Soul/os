#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Вспомогательная функция для ожидания завершения дочернего процесса
void wait_for_child_process() {
    int status;
    if (wait(&status) < 0) { // Ожидаем завершения дочернего процесса
        perror("Ошибка при вызове wait");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) { // Проверяем, завершился ли процесс нормально
        printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) { // Проверяем, был ли процесс завершен сигналом
        printf("Дочерний процесс завершен сигналом: %d\n", WTERMSIG(status));
    }
}

// Функция для демонстрации работы execl()
void demonstrate_execl(const char *path) {
    printf("\n--- Запуск execl ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -l' с использованием execl...\n", getpid());
        // Заменяем образ процесса на команду "ls -l"
        execl(path, "ls", "-l", (char *)NULL);
        // Если execl завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execl");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

// Функция для демонстрации работы execlp()
void demonstrate_execlp(const char *file) {
    printf("\n--- Запуск execlp ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -a' с использованием execlp...\n", getpid());
        // Заменяем образ процесса на команду "ls -a", используя поиск в PATH
        execlp(file, "ls", "-a", (char *)NULL);
        // Если execlp завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execlp");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

// Функция для демонстрации работы execle() с пользовательским окружением
void demonstrate_execle(const char *path, char *const envp[]) {
    printf("\n--- Запуск execle ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -lh' с использованием execle и пользовательского окружения...\n", getpid());
        // Заменяем образ процесса на команду "ls -lh" с пользовательским окружением
        execle(path, "ls", "-lh", (char *)NULL, envp);
        // Если execle завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execle");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

// Функция для демонстрации работы execv()
void demonstrate_execv(const char *path, char *const argv[]) {
    printf("\n--- Запуск execv ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -R' с использованием execv...\n", getpid());
        // Заменяем образ процесса на команду "ls -R"
        execv(path, argv);
        // Если execv завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execv");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

// Функция для демонстрации работы execvp()
void demonstrate_execvp(const char *file, char *const argv[]) {
    printf("\n--- Запуск execvp ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -t' с использованием execvp...\n", getpid());
        // Заменяем образ процесса на команду "ls -t", используя поиск в PATH
        execvp(file, argv);
        // Если execvp завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execvp");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

// Функция для демонстрации работы execve() с пользовательским окружением
void demonstrate_execve(const char *path, char *const argv[], char *const envp[]) {
    printf("\n--- Запуск execve ---\n");
    pid_t pid = fork(); // Создаем дочерний процесс
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Код, выполняемый в дочернем процессе
        printf("Дочерний процесс [%d]: Запуск команды 'ls -la' с использованием execve и пользовательского окружения...\n", getpid());
        // Заменяем образ процесса на команду "ls -la" с пользовательским окружением
        execve(path, argv, envp);
        // Если execve завершился с ошибкой, выводим сообщение
        perror("Ошибка при вызове execve");
        exit(EXIT_FAILURE);
    }
    // Ожидаем завершения дочернего процесса
    wait_for_child_process();
}

int main(void) {
    // Путь к исполняемому файлу и его имя для поиска в PATH
    char *file = "ls";
    char *path = "/bin/ls";

    // Массивы аргументов для запуска команды с разными флагами
    char *args_recursive[] = {"ls", "-R", NULL}; // Рекурсивный вывод
    char *args_time[] = {"ls", "-t", NULL};      // Сортировка по времени
    char *args_long_all[] = {"ls", "-la", NULL}; // Детальный вывод со скрытыми файлами

    // Массив переменных окружения для некоторых вызовов exec*
    char *env[] = {"MY_VAR=123", "PATH=/usr/bin:/bin", NULL};

    printf("Демонстрация работы различных функций семейства exec():\n");

    // Демонстрация execl
    demonstrate_execl(path);

    // Демонстрация execlp
    demonstrate_execlp(file);

    // Демонстрация execle
    demonstrate_execle(path, env);

    // Демонстрация execv
    demonstrate_execv(path, args_recursive);

    // Демонстрация execvp
    demonstrate_execvp(file, args_time);

    // Демонстрация execve
    demonstrate_execve(path, args_long_all, env);

    printf("\nВсе дочерние процессы завершены.\n");
    return 0;
}