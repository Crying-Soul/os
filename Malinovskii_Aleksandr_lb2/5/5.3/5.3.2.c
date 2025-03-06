#include <stdio.h>
#include <sched.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

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
    sched_getparam(pid, &param);

    // Выводим информацию о процессе
    printf("  PID: %d | Приоритет: %d | Политика: %s\n",
           pid, param.sched_priority, get_scheduling_policy(policy));
}

// Функция для поиска процессов с определенной политикой планирования
void find_processes_with_policy(int policy) {
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
        if (atoi(entry->d_name) != 0) {
            pid_t pid = atoi(entry->d_name);

            // Получаем политику планирования для процесса
            int current_policy = sched_getscheduler(pid);
            if (current_policy == policy) {
                print_process_info(pid);  // Выводим информацию о процессе
            }
        }
    }

    closedir(dir);
}

int main() {
    int min_priority, max_priority;

    // Получаем границы приоритетов для SCHED_FIFO
    min_priority = sched_get_priority_min(SCHED_FIFO);
    max_priority = sched_get_priority_max(SCHED_FIFO);
    printf("SCHED_FIFO:\n");
    printf("  Минимальный приоритет = %d\n", min_priority);
    printf("  Максимальный приоритет = %d\n", max_priority);
    printf("  Описание: Политика реального времени с планированием FIFO. Процессы с более высоким приоритетом выполняются первыми.\n");
    printf("  Примеры процессов с SCHED_FIFO:\n");
    find_processes_with_policy(SCHED_FIFO);
    printf("\n");

    // Получаем границы приоритетов для SCHED_RR
    min_priority = sched_get_priority_min(SCHED_RR);
    max_priority = sched_get_priority_max(SCHED_RR);
    printf("SCHED_RR:\n");
    printf("  Минимальный приоритет = %d\n", min_priority);
    printf("  Максимальный приоритет = %d\n", max_priority);
    printf("  Описание: Политика реального времени с планированием Round-Robin. Процессы с одинаковым приоритетом выполняются по очереди с использованием квантов времени.\n");
    printf("  Примеры процессов с SCHED_RR:\n");
    find_processes_with_policy(SCHED_RR);
    printf("\n");

    // Получаем границы приоритетов для SCHED_OTHER
    min_priority = sched_get_priority_min(SCHED_OTHER);
    max_priority = sched_get_priority_max(SCHED_OTHER);
    printf("SCHED_OTHER:\n");
    printf("  Минимальный приоритет = %d\n", min_priority);
    printf("  Максимальный приоритет = %d\n", max_priority);
    printf("  Описание: Стандартная политика планирования Linux с разделением времени. Использует динамические приоритеты на основе значения nice.\n");
    printf("  Примеры процессов с SCHED_OTHER:\n");
    find_processes_with_policy(SCHED_OTHER);
    printf("\n");

    return 0;
}