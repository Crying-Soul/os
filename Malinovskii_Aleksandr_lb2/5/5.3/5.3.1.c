#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>

// Функция для получения политики планирования
const char* get_scheduling_policy(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "UNKNOWN";
    }
}

// Функция, выполняемая в потоке
void* thread_function(void* arg) {
    int policy;
    struct sched_param param;
    int iteration = 0;

    // Устанавливаем политику планирования и приоритет для потока
    policy = SCHED_FIFO;
    param.sched_priority = 50;  // Одинаковый приоритет
    pthread_setschedparam(pthread_self(), policy, &param);

    while (iteration < 5) {
        // Получаем текущие параметры планирования
        pthread_getschedparam(pthread_self(), &policy, &param);

        // Выводим информацию о потоке
        printf("Thread: PID: %d | TID: %ld | Priority: %d | Policy: %s\n",
               getpid(), syscall(SYS_gettid), param.sched_priority, get_scheduling_policy(policy));

        iteration++;
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Устанавливаем политику планирования и приоритет для основного процесса
    struct sched_param param;
    param.sched_priority = 50;  // Одинаковый приоритет
    sched_setscheduler(getpid(), SCHED_FIFO, &param);

    // Создаем два потока
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    // Основной процесс также выводит свою информацию
    int policy;
    int iteration = 0;

    while (iteration < 5) {
        // Получаем текущие параметры планирования
        policy = sched_getscheduler(getpid());
        sched_getparam(getpid(), &param);

        // Выводим информацию о основном процессе
        printf("Main Process: PID: %d | TID: %ld | Priority: %d | Policy: %s\n",
               getpid(), syscall(SYS_gettid), param.sched_priority, get_scheduling_policy(policy));

        iteration++;
    }

    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}