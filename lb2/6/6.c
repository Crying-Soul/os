/**
 * 6.c
 *
 * Функциональность:
 * 1. Комплексное тестирование наследования ресурсов в Linux:
 *    - Файловые дескрипторы (6.1)
 *    - Приоритеты и политики планирования (6.2) 
 *    - Обработчики сигналов (6.3)
 *    - Особенности clone() (6.4)
 *
 * 2. Методы создания процессов:
 *    - fork() - полное копирование
 *    - exec() - замена образа
 *    - clone() - контролируемое наследование
 *
 * 3. Особенности реализации:
 *    - Подробное логирование всех операций
 *    - Проверка ошибок для всех системных вызовов
 *    - Поддержка нескольких режимов работы (основной, exec-потомок)
 *    - Гибкая настройка параметров clone()
 */
#define _GNU_SOURCE  // Для clone()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>
#include <limits.h>
#include <sys/file.h>  // Для файловых блокировок

#define STACK_SIZE (1024 * 1024)

/* Обработчик сигнала для экспериментов */
void signal_handler(int sig) {
    printf("[Сигнал] Процесс (PID %d) получил сигнал %d\n", getpid(), sig);
    fflush(stdout);
}

/* ======== 6.1. Наследование файловых дескрипторов ======== */
void test_file_descriptors_fork(void) {
    printf("=== Тест 6.1.1: Наследование файловых дескрипторов через fork() ===\n");
    fflush(stdout);

    int fd = open("testfile_fork.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("Ошибка открытия файла");
        exit(EXIT_FAILURE);
    }

    const char *parent_line1 = "Родитель записывает строку 1 (fork)\n";
    if (write(fd, parent_line1, strlen(parent_line1)) < 0) {
        perror("Ошибка записи (родитель, строка 1)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        const char *child_line = "Потомок записывает строку 2 (fork)\n";
        if (write(fd, child_line, strlen(child_line)) < 0) {
            perror("Ошибка записи (потомок)");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("[Потомок] PID %d: Данные записаны в 'testfile_fork.txt'\n", getpid());
        fflush(stdout);
        close(fd);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        const char *parent_line2 = "Родитель записывает строку 3 (fork)\n";
        if (write(fd, parent_line2, strlen(parent_line2)) < 0) {
            perror("Ошибка записи (родитель, строка 3)");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("[Родитель] PID %d: Данные записаны в 'testfile_fork.txt'\n", getpid());
        fflush(stdout);
        close(fd);
    }

    printf("Проверьте содержимое файла 'testfile_fork.txt'.\n\n");
    fflush(stdout);
}

void test_file_descriptors_exec(char *argv[]) {
    printf("=== Тест 6.1.2: Наследование файловых дескрипторов через exec() ===\n");
    fflush(stdout);

    int fd = open("testfile_exec.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("Ошибка открытия файла (exec)");
        exit(EXIT_FAILURE);
    }

    const char *parent_line = "Родитель записывает строку перед exec\n";
    if (write(fd, parent_line, strlen(parent_line)) < 0) {
        perror("Ошибка записи (родитель, перед exec)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Потомок: перенаправляем файловый дескриптор
        if (dup2(fd, 3) == -1) {
            perror("Ошибка dup2");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);

        // Запуск новой программы через exec
        execl(argv[0], argv[0], "exec_child", NULL);
        perror("Ошибка execl");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
        close(fd);
    }

    printf("Тест exec завершён. Проверьте содержимое файла 'testfile_exec.txt'.\n\n");
    fflush(stdout);
}
int clone_child_function(void *arg) {
    int *fd = (int *)arg;
    printf("[Clone-потомок] PID %d: Запущен. Использую файловый дескриптор: %d\n", getpid(), *fd);
    fflush(stdout);

    const char *clone_line = "Clone-потомок записывает строку\n";
    if (write(*fd, clone_line, strlen(clone_line)) < 0) {
        perror("Ошибка записи (clone-потомок)");
        exit(EXIT_FAILURE);
    }
    return 0;
}


void test_file_descriptors_clone(void) {
    printf("=== Тест 6.1.3: Наследование файловых дескрипторов через clone() ===\n");
    fflush(stdout);

    int fd = open("testfile_clone_fd.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("Ошибка открытия файла (clone)");
        exit(EXIT_FAILURE);
    }

    const char *parent_line = "Родитель записывает строку перед clone\n";
    if (write(fd, parent_line, strlen(parent_line)) < 0) {
        perror("Ошибка записи (родитель, перед clone)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("Ошибка malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    int clone_flags = SIGCHLD | CLONE_FILES;
    pid_t child_pid = clone(clone_child_function, stack_top, clone_flags, &fd);
    if (child_pid == -1) {
        perror("Ошибка clone");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    waitpid(child_pid, NULL, 0);

    const char *parent_after = "Родитель записывает строку после clone\n";
    if (write(fd, parent_after, strlen(parent_after)) < 0) {
        perror("Ошибка записи (родитель, после clone)");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    free(stack);

    printf("Тест clone (файловые дескрипторы) завершён. Проверьте содержимое файла 'testfile_clone_fd.txt'.\n\n");
    fflush(stdout);
}

/* ======== 6.2. Наследование приоритетов и политики планирования ======== */
void test_priority_and_scheduling_fork(void) {
    printf("=== Тест 6.2.1: Наследование приоритетов и политики планирования через fork() ===\n");
    fflush(stdout);

    if (nice(5) == -1 && errno != 0) {
        perror("Ошибка nice");
    }
    int prio = getpriority(PRIO_PROCESS, 0);
    int policy = sched_getscheduler(0);
    printf("[Родитель] PID %d: Приоритет = %d, Политика планирования = %d\n", getpid(), prio, policy);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int child_prio = getpriority(PRIO_PROCESS, 0);
        int child_policy = sched_getscheduler(0);
        printf("[Потомок] PID %d: Приоритет = %d, Политика планирования = %d\n", getpid(), child_prio, child_policy);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }

    printf("Тест наследования приоритетов через fork() завершён.\n\n");
    fflush(stdout);
}

void test_priority_and_scheduling_exec(char *argv[]) {
    printf("=== Тест 6.2.2: Наследование приоритетов и политики планирования через exec() ===\n");
    fflush(stdout);

    // Устанавливаем политику планирования SCHED_FIFO и приоритет 99
    struct sched_param param;
    param.sched_priority = 99; // Устанавливаем приоритет 99

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("Ошибка sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    // Проверяем, что политика и приоритет установлены
    int policy = sched_getscheduler(0);
    if (policy == -1) {
        perror("Ошибка sched_getscheduler");
        exit(EXIT_FAILURE);
    }

    if (sched_getparam(0, &param) == -1) {
        perror("Ошибка sched_getparam");
        exit(EXIT_FAILURE);
    }

    printf("[Родитель] PID %d: Приоритет = %d, Политика планирования = %d\n", 
           getpid(), param.sched_priority, policy);
    fflush(stdout);

    // Устанавливаем nice-значение (оно не влияет на реальные приоритеты для SCHED_FIFO)
    if (nice(5) == -1 && errno != 0) {
        perror("Ошибка nice");
    }

    // Создаем дочерний процесс
    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // В дочернем процессе вызываем exec()
        execl(argv[0], argv[0], "exec_child_priority", NULL);
        perror("Ошибка execl");
        exit(EXIT_FAILURE);
    } else {
        // В родительском процессе ждем завершения дочернего
        wait(NULL);
    }

    printf("Тест наследования приоритетов через exec() завершён.\n\n");
    fflush(stdout);
}

int clone_child_priority_function(void *arg) {
    printf("[Clone-потомок] PID %d: Запущен. Проверка наследования приоритетов и политики планирования.\n", getpid());
    fflush(stdout);

    int prio = getpriority(PRIO_PROCESS, 0);
    int policy = sched_getscheduler(0);
    printf("[Clone-потомок] PID %d: Приоритет = %d, Политика планирования = %d\n", getpid(), prio, policy);
    fflush(stdout);

    return 0;
}

void test_priority_and_scheduling_clone(void) {
    printf("=== Тест 6.2.3: Наследование приоритетов и политики планирования через clone() ===\n");
    fflush(stdout);

    // Устанавливаем политику планирования SCHED_FIFO и приоритет 99
    struct sched_param param;
    param.sched_priority = 99;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("Ошибка sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    int policy = sched_getscheduler(0);
    if (policy == -1) {
        perror("Ошибка sched_getscheduler");
        exit(EXIT_FAILURE);
    }

    if (sched_getparam(0, &param) == -1) {
        perror("Ошибка sched_getparam");
        exit(EXIT_FAILURE);
    }

    printf("[Родитель] PID %d: Приоритет = %d, Политика планирования = %d\n", 
           getpid(), param.sched_priority, policy);
    fflush(stdout);

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("Ошибка malloc");
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    int clone_flags = SIGCHLD;
    pid_t child_pid = clone(clone_child_priority_function, stack_top, clone_flags, NULL);
    if (child_pid == -1) {
        perror("Ошибка clone");
        free(stack);
        exit(EXIT_FAILURE);
    }

    waitpid(child_pid, NULL, 0);
    free(stack);

    printf("Тест clone (приоритеты и политика планирования) завершён.\n\n");
    fflush(stdout);
}

/* ======== 6.3. Наследование сигналов ======== */
void test_signal_handling_fork(void) {
    printf("=== Тест 6.3.1: Наследование сигналов через fork() ===\n");
    fflush(stdout);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        sleep(1);
        printf("[Потомок] PID %d: Отправляю SIGUSR1 себе\n", getpid());
        fflush(stdout);
        raise(SIGUSR1);
        exit(EXIT_SUCCESS);
    } else {
        sleep(2);
        printf("[Родитель] PID %d: Отправляю SIGUSR1 потомку (PID %d)\n", getpid(), pid);
        fflush(stdout);
        raise(SIGUSR1);
        wait(NULL);
    }

    printf("Тест наследования сигналов через fork() завершён.\n\n");
    fflush(stdout);
}

void test_signal_handling_exec(char *argv[]) {
    printf("=== Тест 6.3.2: Наследование сигналов через exec() ===\n");
    fflush(stdout);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execl(argv[0], argv[0], "exec_child_signal", NULL);
        perror("Ошибка execl");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }

    printf("Тест наследования сигналов через exec() завершён.\n\n");
    fflush(stdout);
}

int clone_child_signal_function(void *arg) {
    printf("[Clone-потомок] PID %d: Запущен. Проверка наследования сигналов.\n", getpid());
    fflush(stdout);

    struct sigaction sa;
    if (sigaction(SIGUSR1, NULL, &sa) == -1) {
        perror("Ошибка sigaction (clone-потомок)");
    } else {
        if (sa.sa_handler == signal_handler) {
            printf("[Clone-потомок] PID %d: Обработчик SIGUSR1 унаследован.\n", getpid());
        } else {
            printf("[Clone-потомок] PID %d: Обработчик SIGUSR1 НЕ унаследован (неожиданно).\n", getpid());
        }
        fflush(stdout);
    }

    printf("[Clone-потомок] PID %d: Посылаю SIGUSR1 самому себе.\n", getpid());
    fflush(stdout);
    raise(SIGUSR1);

    return 0;
}

void test_signal_handling_clone(void) {
    printf("=== Тест 6.3.3: Наследование сигналов через clone() ===\n");
    fflush(stdout);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(EXIT_FAILURE);
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("Ошибка malloc");
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    int clone_flags = SIGCHLD;
    pid_t child_pid = clone(clone_child_signal_function, stack_top, clone_flags, NULL);
    if (child_pid == -1) {
        perror("Ошибка clone");
        free(stack);
        exit(EXIT_FAILURE);
    }

    waitpid(child_pid, NULL, 0);
    free(stack);

    printf("Тест clone (сигналы) завершён.\n\n");
    fflush(stdout);
}

/* ======== 6.4. Наследование для clone() ======== */


void test_clone_inheritance(void) {
    printf("=== Тест 6.4: Наследование для clone() ===\n");
    fflush(stdout);

    int fd = open("testfile_clone.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("Ошибка открытия файла (clone)");
        exit(EXIT_FAILURE);
    }

    const char *parent_before = "Родитель (до clone) записывает строку\n";
    if (write(fd, parent_before, strlen(parent_before)) < 0) {
        perror("Ошибка записи (родитель, до clone)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("Ошибка malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    int clone_flags = SIGCHLD | CLONE_FILES;
    pid_t child_pid = clone(clone_child_function, stack_top, clone_flags, &fd);
    if (child_pid == -1) {
        perror("Ошибка clone");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    waitpid(child_pid, NULL, 0);

    const char *parent_after = "Родитель (после clone) записывает строку\n";
    if (write(fd, parent_after, strlen(parent_after)) < 0) {
        perror("Ошибка записи (родитель, после clone)");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    free(stack);

    printf("Тест clone завершён. Проверьте содержимое файла 'testfile_clone.txt'.\n\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "exec_child") == 0) {
        printf("=== Exec Child Process ===\n");
        printf("[Exec-потомок] PID %d: Запущен через exec(), проверка наследования.\n", getpid());
        fflush(stdout);

        struct sigaction sa;
        if (sigaction(SIGUSR1, NULL, &sa) == -1) {
            perror("Ошибка sigaction (exec-потомок)");
        } else {
            if (sa.sa_handler == SIG_DFL) {
                printf("[Exec-потомок] PID %d: Обработчик SIGUSR1 сброшен к значению по умолчанию.\n", getpid());
            } else {
                printf("[Exec-потомок] PID %d: Обработчик SIGUSR1 НЕ сброшен (неожиданно).\n", getpid());
            }
            fflush(stdout);
        }

        const char *exec_line = "Exec-потомок записывает строку через FD 3\n";
        if (write(3, exec_line, strlen(exec_line)) < 0) {
            perror("Ошибка записи (exec-потомок)");
        } else {
            printf("[Exec-потомок] PID %d: Данные записаны в файл через FD 3.\n", getpid());
        }
        fflush(stdout);

        printf("[Exec-потомок] PID %d: Посылаю SIGUSR1 самому себе.\n", getpid());
        fflush(stdout);
        raise(SIGUSR1);

        return 0;
    } else if (argc > 1 && strcmp(argv[1], "exec_child_priority") == 0) {
        printf("=== Exec Child Process (Priority) ===\n");
        printf("[Exec-потомок] PID %d: Запущен через exec(), проверка наследования приоритетов.\n", getpid());
        fflush(stdout);

        int prio = getpriority(PRIO_PROCESS, 0);
        int policy = sched_getscheduler(0);
        printf("[Exec-потомок] PID %d: Приоритет = %d, Политика планирования = %d\n", getpid(), prio, policy);
        fflush(stdout);

        return 0;
    } else if (argc > 1 && strcmp(argv[1], "exec_child_signal") == 0) {
        printf("=== Exec Child Process (Signal) ===\n");
        printf("[Exec-потомок] PID %d: Запущен через exec(), проверка наследования сигналов.\n", getpid());
        fflush(stdout);

        struct sigaction sa;
        if (sigaction(SIGUSR1, NULL, &sa) == -1) {
            perror("Ошибка sigaction (exec-потомок)");
        } else {
            if (sa.sa_handler == SIG_DFL) {
                printf("[Exec-потомок] PID %d: Обработчик SIGUSR1 сброшен к значению по умолчанию.\n", getpid());
            } else {
                printf("[Exec-потомок] PID %d: Обработчик SIGUSR1 НЕ сброшен (неожиданно).\n", getpid());
            }
            fflush(stdout);
        }

        printf("[Exec-потомок] PID %d: Посылаю SIGUSR1 самому себе.\n", getpid());
        fflush(stdout);
        raise(SIGUSR1);

        return 0;
    } else {
        printf("=== Демонстрация наследования ресурсов в Linux (fork, exec, clone) ===\n");
        printf("[Родитель] PID %d: Запущен.\n\n", getpid());
        fflush(stdout);

        // 6.1. Наследование файловых дескрипторов
        test_file_descriptors_fork();
        test_file_descriptors_exec(argv);
        //test_file_descriptors_clone();

        // 6.2. Наследование приоритетов и политики планирования
        test_priority_and_scheduling_fork();
        test_priority_and_scheduling_exec(argv);
        test_priority_and_scheduling_clone();

        // 6.3. Наследование сигналов
        test_signal_handling_fork();
        test_signal_handling_exec(argv);
        test_signal_handling_clone();

        // 6.4. Наследование для clone()
        test_clone_inheritance();

        printf("Все тесты завершены. Проверьте созданные файлы:\n");
        printf(" - testfile_fork.txt\n - testfile_exec.txt\n - testfile_clone.txt\n");
        fflush(stdout);
    }

    return 0;
}