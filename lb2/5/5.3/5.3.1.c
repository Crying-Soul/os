#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_PRIORITY 50
#define ITERATIONS 5

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

// Функция для вывода информации о процессе или потоке
void print_thread_info(const char *thread_type, pid_t pid, int policy,
                       struct sched_param param) {
  printf("%-7s | PID: %-3d | TID: %-4ld | Priority: %-3d | Policy: %-12s\n",
         thread_type, pid, syscall(SYS_gettid), param.sched_priority,
         get_scheduling_policy(policy));
}

// Функция, выполняемая потоками
void *thread_function(void *arg) {
  struct sched_param param = {.sched_priority = FIFO_PRIORITY};

  // Устанавливаем политику и приоритет для текущего потока
  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
    perror("pthread_setschedparam");
    return NULL;
  }

  int policy;
  for (int iteration = 0; iteration < ITERATIONS; iteration++) {
    pthread_getschedparam(pthread_self(), &policy, &param);
    print_thread_info("Thread", getpid(), policy, param);
  }

  return NULL;
}

int main() {
  struct sched_param main_param = {.sched_priority = FIFO_PRIORITY};

  // Устанавливаем политику и приоритет для основного потока
  if (sched_setscheduler(getpid(), SCHED_FIFO, &main_param) != 0) {
    perror("sched_setscheduler");
    return 1;
  }

  pthread_t thread1, thread2;

  // Создаем потоки
  if (pthread_create(&thread1, NULL, thread_function, NULL) != 0) {
    perror("pthread_create thread1");
    return 1;
  }
  if (pthread_create(&thread2, NULL, thread_function, NULL) != 0) {
    perror("pthread_create thread2");
    return 1;
  }

  int policy;
  for (int iteration = 0; iteration < ITERATIONS; iteration++) {
    policy = sched_getscheduler(getpid());
    sched_getparam(getpid(), &main_param);
    print_thread_info("Main", getpid(), policy, main_param);
  }

  // Ожидаем завершения потоков
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}
