#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// Функция для получения политики планирования
const char *get_scheduling_policy(int policy) {
  switch (policy) {
  case SCHED_OTHER:
    return "SCHED_OTHER";
  case SCHED_FIFO:
    return "SCHED_FIFO";
  case SCHED_RR:
    return "SCHED_RR";
  default:
    return "UNKNOWN";
  }
}

// Функция, выполняемая в потоке
void *thread_function(void *arg) {
  int priority = *((int *)arg); // Получаем приоритет из аргумента
  int policy;
  struct sched_param param;
  int iteration = 0;

  // Устанавливаем политику планирования и приоритет для потока
  policy = SCHED_RR;
  param.sched_priority = priority;
  pthread_setschedparam(pthread_self(), policy, &param);

  while (iteration < 5) {
    // Получаем текущие параметры планирования
    pthread_getschedparam(pthread_self(), &policy, &param);

    // Выводим информацию о потоке
    printf("Thread: PID: %d | TID: %ld | Priority: %d | Policy: %s\n", getpid(),
           syscall(SYS_gettid), param.sched_priority,
           get_scheduling_policy(policy));

    iteration++;

    // Используем sched_yield() для изменения порядка очереди
    if (iteration == 2) {
      printf("Thread (PID: %d) вызывает sched_yield()\n", getpid());
      sched_yield(); // Передача управления следующему потоку
    }
  }
  return NULL;
}

int main() {
  pthread_t thread1, thread2, thread3;
  int priority1 = 20, priority2 = 20, priority3 = 20; // Приоритеты для потоков

  // Устанавливаем политику планирования и приоритет для основного процесса
  struct sched_param param;
  param.sched_priority = 40; // Приоритет основного процесса
  sched_setscheduler(getpid(), SCHED_RR, &param);

  // Создаем три потока с разными приоритетами
  pthread_create(&thread1, NULL, thread_function, &priority1);
  pthread_create(&thread2, NULL, thread_function, &priority2);
  pthread_create(&thread3, NULL, thread_function, &priority3);

  // Основной процесс также выводит свою информацию
  int policy;
  int iteration = 0;

  while (iteration < 5) {
    // Получаем текущие параметры планирования
    policy = sched_getscheduler(getpid());
    sched_getparam(getpid(), &param);

    // Выводим информацию о основном процессе
    printf("Main Process: PID: %d | TID: %ld | Priority: %d | Policy: %s\n",
           getpid(), syscall(SYS_gettid), param.sched_priority,
           get_scheduling_policy(policy));

    iteration++;
  }

  // Ожидаем завершения потоков
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);

  return 0;
}