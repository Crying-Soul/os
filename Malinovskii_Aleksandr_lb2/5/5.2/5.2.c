#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>

// Функция для получения и вывода текущего приоритета процесса
void print_priority(const char *process_name, pid_t pid) {
    int prio = getpriority(PRIO_PROCESS, pid); // Получаем приоритет для указанного PID
    if (prio == -1 && errno != 0) {
        perror("Ошибка при получении приоритета");
    } else {
        printf("%-40s (PID: %d): Текущий приоритет (значение nice): %d\n", process_name, pid, prio);
    }
}

int main() {
    // Пример системного процесса (например, демон cron)
    //pid_t system_process_pid = 10; 
    //printf("=== Сравнение и изменение приоритетов системного и пользовательского процессов ===\n\n");

    // Получаем и выводим приоритет системного процесса
    //printf("[Системный процесс]\n");
    //print_priority("Системный процесс (systemd/init)", system_process_pid);

    // Пытаемся изменить приоритет системного процесса
   // printf("\nПытаемся изменить приоритет системного процесса (PID: %d)...\n", system_process_pid);
    //if (setpriority(PRIO_PROCESS, system_process_pid, 10) == -1) {
     //   perror("Ошибка setpriority для системного процесса (вероятно, из-за отсутствия прав)");
    //} else {
    //    printf("Приоритет системного процесса изменен.\n");
    //}

    // Проверяем приоритет системного процесса после попытки изменения
    //print_priority("Системный процесс после попытки изменения", system_process_pid);

    // Родительский процесс (пользовательский)
    printf("\n[Пользовательский процесс]\n");
    print_priority("Родительский процесс (пользовательский)", getpid());

    pid_t pid = fork(); // Создаем дочерний процесс

    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса (fork)");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("\n[Дочерний процесс]\n");
        print_priority("Дочерний процесс до изменения nice", getpid());

        // Пытаемся увеличить приоритет (уменьшить значение nice)
        printf("\nПытаемся установить значение nice на -10...\n");
        if (nice(-10) == -1) {
            perror("Ошибка nice (вероятно, из-за отсутствия прав)");
        }

        print_priority("Дочерний процесс после изменения nice", getpid());

        // Пытаемся уменьшить приоритет (увеличить значение nice)
        printf("\nПытаемся установить значение nice на 10...\n");
        if (nice(10) == -1) {
            perror("Ошибка nice");
        }

        print_priority("Дочерний процесс после изменения nice", getpid());

        exit(EXIT_SUCCESS); // Завершаем дочерний процесс
    } else {
        // Родительский процесс
        printf("\n[Родительский процесс]\n");
        print_priority("Родительский процесс до изменения приоритета", getpid());

        // Пытаемся увеличить приоритет (уменьшить значение nice)
        printf("\nПытаемся установить приоритет на -10...\n");
        if (setpriority(PRIO_PROCESS, getpid(), -10) == -1) {
            perror("Ошибка setpriority (вероятно, из-за отсутствия прав)");
        }

        print_priority("Родительский процесс после изменения приоритета", getpid());

        // Пытаемся уменьшить приоритет (увеличить значение nice)
        printf("\nПытаемся установить приоритет на 10...\n");
        if (setpriority(PRIO_PROCESS, getpid(), 10) == -1) {
            perror("Ошибка setpriority");
        }

        print_priority("Родительский процесс после изменения приоритета", getpid());

        // Ожидаем завершения дочернего процесса
        wait(NULL);
    }

    printf("\n=== Завершение программы ===\n");
    return 0;
}