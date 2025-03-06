#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 32

struct thread_params {
  int priority;
  const char *name;
};

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

void *thread_function(void *arg) {
  struct thread_params *params = (struct thread_params *)arg;
  int policy;
  struct sched_param param;
  int iteration = 0;

  while (iteration < 5) {
    pthread_getschedparam(pthread_self(), &policy, &param);

    printf("%-7s | PID: %-5d | TID: %-5ld | Priority: %-3d | Policy: %-12s\n",
           params->name, getpid(), syscall(SYS_gettid), param.sched_priority,
           get_scheduling_policy(policy));

    iteration++;
  }
  return NULL;
}

int main() {
  pthread_attr_t attr_main, attr1, attr2;
  struct sched_param param_main, param1, param2;

  // Настройка главного потока
  pthread_attr_init(&attr_main);
  pthread_attr_setinheritsched(&attr_main, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr_main, SCHED_FIFO);
  param_main.sched_priority = 50;
  pthread_attr_setschedparam(&attr_main, &param_main);
  pthread_setschedparam(pthread_self(), SCHED_FIFO, &param_main);

  struct thread_params params1 = {.priority = 90, .name = "Thread1"};
  struct thread_params params2 = {.priority = 30, .name = "Thread2"};

  // Настройка атрибутов для потоков
  pthread_attr_init(&attr1);
  pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
  param1.sched_priority = params1.priority;
  pthread_attr_setschedparam(&attr1, &param1);

  pthread_attr_init(&attr2);
  pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
  param2.sched_priority = params2.priority;
  pthread_attr_setschedparam(&attr2, &param2);

  pthread_t thread1, thread2;
  if (pthread_create(&thread1, &attr1, thread_function, &params1) != 0) {
    fprintf(stderr, "Ошибка создания потока 1: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  if (pthread_create(&thread2, &attr2, thread_function, &params2) != 0) {
    fprintf(stderr, "Ошибка создания потока 2: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // Вывод информации из главного потока
  char timestamp[20];
  int iteration = 0;
  while (iteration < 5) {

    printf("%-7s | PID: %-5d | TID: %-5ld | Priority: %-3d | Policy: %-12s\n",
           "Main", getpid(), syscall(SYS_gettid), param_main.sched_priority,
           get_scheduling_policy(SCHED_FIFO));
    iteration++;
  }

  pthread_attr_destroy(&attr_main);
  pthread_attr_destroy(&attr1);
  pthread_attr_destroy(&attr2);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}