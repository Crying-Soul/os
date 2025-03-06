#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

// Функция для получения политики планирования
const char* get_scheduling_policy(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "UNKNOWN";
    }
}

// Функция для вывода информации о процессе
void print_process_info(pid_t pid) {
    int policy;
    struct sched_param param;

    // Получаем политику планирования и параметры
    policy = sched_getscheduler(pid);
    if (policy == -1) {
        perror("sched_getscheduler");
        return;
    }

    if (sched_getparam(pid, &param) == -1) {
        perror("sched_getparam");
        return;
    }

    // Выводим информацию
    printf("PID: %d Nice: %d | Policy: %s\n",
           pid, param.sched_priority, get_scheduling_policy(policy));
}

// Функция для проверки, является ли строка числом
int is_number(const char* str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

// Функция для чтения списка процессов из /proc
void read_process_table() {
    DIR *dir;
    struct dirent *entry;

    // Открываем директорию /proc
    dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return;
    }

    // Читаем записи в директории
    while ((entry = readdir(dir)) != NULL) {
        // Проверяем, является ли имя директории числом (PID)
        if (is_number(entry->d_name)) {
            pid_t pid = atoi(entry->d_name);
            print_process_info(pid);  // Выводим информацию о процессе
        }
    }

    closedir(dir);
}

// Функция для запуска процесса по его имени
void launch_process(const char* program) {
    pid_t pid = fork();
    if (pid == 0) {
        // Дочерний процесс
        execlp(program, program, NULL);
        perror("execlp");  // Если execlp fails
        exit(1);
    } else if (pid > 0) {
        // Родительский процесс
        printf("Launched process %s with PID: %d\n", program, pid);
    } else {
        perror("fork");
    }
}

int main() {
    // Читаем и выводим информацию о процессах из таблицы процессов
    printf("Process table:\n");
    read_process_table();

    // Основной процесс также выводит свою информацию
    printf("\nMain process:\n");
    print_process_info(getpid());

    // Запускаем процессы из таблицы процессов
    printf("\nLaunching processes...\n");
    launch_process("ls");  

    // Ждем завершения дочерних процессов
    while (wait(NULL) > 0);

    return 0;
}