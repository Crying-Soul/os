/*
 * improved_inheritance_example.c
 *
 * Демонстрация наследования ресурсов в Linux:
 * - Наследование файловых дескрипторов.
 * - Наследование приоритетов и политики планирования.
 * - Наследование сигналов и маски сигналов.
 * - Наследование лимитов ресурсов.
 * - Наследование переменных окружения.
 * - Наследование текущей рабочей директории.
 * - Примеры с fork(), exec() и clone() с флагом CLONE_FILES.
 *
 * В эксперименте с exec() дочерний процесс вызывает exec() этой же программы с аргументом "exec_child".
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

#define STACK_SIZE (1024 * 1024)

/* Обработчик сигнала для экспериментов */
void signal_handler(int sig) {
    printf("Процесс (PID %d) получил сигнал %d\n", getpid(), sig);
    fflush(stdout);
}

/* ======== 6.1. Наследование файловых дескрипторов ======== */
void test_file_inheritance(void) {
    printf("=== Тест 6.1: Наследование файловых дескрипторов ===\n");
    fflush(stdout);

    int fd = open("testfile.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    const char *parent_line1 = "Parent writes line 1\n";
    if (write(fd, parent_line1, strlen(parent_line1)) < 0) {
        perror("write (parent line 1)");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        const char *child_line = "Child writes line 2\n";
        if (write(fd, child_line, strlen(child_line)) < 0) {
            perror("write (child)");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("Child (PID %d): Записал данные в 'testfile.txt'\n", getpid());
        fflush(stdout);
        close(fd);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        const char *parent_line2 = "Parent writes line 3\n";
        if (write(fd, parent_line2, strlen(parent_line2)) < 0) {
            perror("write (parent line 3)");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("Parent (PID %d): Записал данные в 'testfile.txt'\n", getpid());
        fflush(stdout);
        close(fd);
    }
    
    printf("Проверьте содержимое файла 'testfile.txt'.\n\n");
    fflush(stdout);
}

/* ======== 6.2. Наследование приоритетов и политики планирования ======== */
void test_priority_inheritance(void) {
    printf("=== Тест 6.2: Наследование приоритетов и политики планирования ===\n");
    fflush(stdout);

    if (nice(5) == -1 && errno != 0) {
        perror("nice");
    }
    int prio = getpriority(PRIO_PROCESS, 0);
    int policy = sched_getscheduler(0);
    printf("Родитель (PID %d): приоритет = %d, политика планирования = %d\n", getpid(), prio, policy);
    fflush(stdout);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        int child_prio = getpriority(PRIO_PROCESS, 0);
        int child_policy = sched_getscheduler(0);
        printf("Потомок (PID %d): приоритет = %d, политика планирования = %d\n", getpid(), child_prio, child_policy);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }
    
    printf("Завершён тест наследования приоритетов.\n\n");
    fflush(stdout);
}

/* ======== 6.3. Наследование сигналов (без exec) ======== */
void test_signal_inheritance(void) {
    printf("=== Тест 6.3: Наследование сигналов (без exec) ===\n");
    fflush(stdout);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        sleep(1);
        printf("Потомок (PID %d): Отправляет SIGUSR1 себе\n", getpid());
        fflush(stdout);
        raise(SIGUSR1);
        exit(EXIT_SUCCESS);
    } else {
        sleep(2);
        printf("Родитель (PID %d): Отправляет SIGUSR1 потомку (PID %d)\n", getpid(), pid);
        fflush(stdout);
        raise(SIGUSR1);
        // kill(pid, SIGUSR1);
        wait(NULL);
    }
    
    printf("Завершён тест наследования сигналов (без exec).\n\n");
    fflush(stdout);
}


/* ======== 6.4. Наследование при clone() ======== */
/* Функция, запускаемая clone-потомком */
int clone_func(void *arg) {
    int *fd = (int *)arg;
    printf("Clone-потомок (PID %d) запущен. Используем файловый дескриптор: %d\n", getpid(), *fd);
    fflush(stdout);
    
    const char *clone_line = "Clone child writes line\n";
    if (write(*fd, clone_line, strlen(clone_line)) < 0) {
        perror("write (clone child)");
        exit(EXIT_FAILURE);
    }
    return 0;
}

void test_clone_inheritance(void) {
    printf("=== Тест 6.4: Наследование в clone() ===\n");
    fflush(stdout);

    int fd = open("testfile_clone.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0) {
        perror("open (clone test)");
        exit(EXIT_FAILURE);
    }
    
    const char *parent_before = "Parent (before clone) writes line\n";
    if (write(fd, parent_before, strlen(parent_before)) < 0) {
        perror("write (parent before clone)");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;
    
    int clone_flags = SIGCHLD | CLONE_FILES;
    pid_t child_pid = clone(clone_func, stack_top, clone_flags, &fd);
    if (child_pid == -1) {
        perror("clone");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    waitpid(child_pid, NULL, 0);
    
    const char *parent_after = "Parent (after clone) writes line\n";
    if (write(fd, parent_after, strlen(parent_after)) < 0) {
        perror("write (parent after clone)");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    free(stack);
    
    printf("Завершён тест clone наследования. Проверьте содержимое файла 'testfile_clone.txt'.\n\n");
    fflush(stdout);
}
/*
 * Точка входа программы.
 * Если программа запущена с аргументом "exec_child", выполняется ветка для демонстрации
 * поведения после exec(), где проверяется, что обработчики сигналов сброшены.
 */
int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "exec_child") == 0) {
        printf("=== Exec Child Process ===\n");
        printf("Exec Child (PID %d): Запущен через exec(), проверка наследования.\n", getpid());
        fflush(stdout);
        
        struct sigaction sa;
        if (sigaction(SIGUSR1, NULL, &sa) == -1) {
            perror("sigaction (exec child)");
        } else {
            if (sa.sa_handler == SIG_DFL) {
                printf("Exec Child (PID %d): Обработчик SIGUSR1 сброшен к значению по умолчанию.\n", getpid());
            } else {
                printf("Exec Child (PID %d): Обработчик SIGUSR1 НЕ сброшен (неожиданно).\n", getpid());
            }
            fflush(stdout);
        }
        
        const char *exec_line = "Exec child writes line via FD 3\n";
        if (write(3, exec_line, strlen(exec_line)) < 0) {
            perror("write (exec child)");
        } else {
            printf("Exec Child (PID %d): Записал данные в файл через FD 3.\n", getpid());
        }
        fflush(stdout);
        
        printf("Exec Child (PID %d): Посылаю SIGUSR1 самому себе.\n", getpid());
        fflush(stdout);
        raise(SIGUSR1);
        
        return 0;
    } else {
        printf("Демонстрация наследования ресурсов в Linux (fork, exec, clone)\n");
        printf("Запущен процесс (PID %d).\n\n", getpid());
        fflush(stdout);

        test_file_inheritance();
        test_priority_inheritance();
        test_signal_inheritance();
       
        test_clone_inheritance();
        
        printf("Все тесты завершены. Проверьте созданные файлы:\n");
        printf(" - testfile.txt\n - testfile_exec.txt\n - testfile_clone.txt\n");
        fflush(stdout);
    }
    
    return 0;
}
