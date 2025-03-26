/**
 * 4.c
 * 
 * Функциональность:
 * 1. Демонстрация двух подходов к созданию потоков:
 *    - Традиционные POSIX-потоки (pthread_create)
 *    - Низкоуровневые потоки через clone()
 * 
 * 2. Управление потоками:
 *    - Единый обработчик сигналов для всех потоков
 *    - Глобальный флаг остановки (stop_flag)
 *    - Корректное завершение по сигналу (SIGINT/SIGTERM)
 * 
 * 3. Особенности реализации:
 *    - Выделение отдельных стеков для clone-потоков
 *    - Комбинирование флагов CLONE_* для разной степени изоляции
 *    - Получение реальных TID через syscall(SYS_gettid)
 *    - Гарантированный сброс буферов вывода
 * 
 * 4. Диагностика:
 *    - Подробное логирование состояния потоков
 *    - Ожидание завершения всех потоков
 *    - Очистка ресурсов
 */
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024) // 1 МБ стека для каждого потока
#define NUM_THREADS 3

volatile sig_atomic_t stop_flag = 0; // Флаг для остановки потоков

// Обработчик сигналов
void signal_handler(int sig) {
  stop_flag = 1;
  printf("Received signal %d, stopping threads...\n", sig);
  fflush(stdout); // Сбрасываем буфер
}

// Функция для потоков, созданных через clone()
int clone_thread(void *arg) {
  int id = *(int *)arg;
  while (!stop_flag) {
    printf("Clone thread %d (TID: %ld)\n", id, (long)syscall(SYS_gettid));
    fflush(stdout); // Сбрасываем буфер
    sleep(1);
  }
  printf("Clone thread %d exiting\n", id);
  fflush(stdout); // Сбрасываем буфер
  return 0;
}
  
// Функция для потоков, созданных через pthread
void *pthread_thread(void *arg) {
  int id = *(int *)arg;
  while (!stop_flag) {
    printf("Pthread thread %d (TID: %ld)\n", id, (long)syscall(SYS_gettid));
    fflush(stdout); // Сбрасываем буфер
    sleep(1);
  }
  printf("Pthread thread %d exiting\n", id);
  fflush(stdout); // Сбрасываем буфер
  return NULL;
}

int main(void) {
  // Настройка обработчика сигналов
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1 ||
      sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction failed");
    exit(EXIT_FAILURE);
  }

  pthread_t pthreads[NUM_THREADS];
  int ids[NUM_THREADS * 2]; // Идентификаторы для всех потоков
  char *stacks[NUM_THREADS];
  pid_t clone_pids[NUM_THREADS];

  // Инициализация идентификаторов
  for (int i = 0; i < NUM_THREADS * 2; i++) {
    ids[i] = i + 1;
  }

  // Создание потоков через pthread_create()
  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_create(&pthreads[i], NULL, pthread_thread, &ids[i]) != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  // Создание потоков через clone()
  for (int i = 0; i < NUM_THREADS; i++) {
    stacks[i] = malloc(STACK_SIZE);
    if (!stacks[i]) {
      perror("malloc failed");
      exit(EXIT_FAILURE);
    }

    clone_pids[i] = clone(clone_thread, stacks[i] + STACK_SIZE,
                          CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                              CLONE_THREAD | CLONE_SYSVSEM,
                          &ids[i + NUM_THREADS]);

    if (clone_pids[i] == -1) {
      fprintf(stderr, "clone failed: %s\n", strerror(errno));
      free(stacks[i]);
      exit(EXIT_FAILURE);
    }
  }

  // Ожидание сигнала для завершения
  while (!stop_flag) {
    pause();
  }

  // Ожидание завершения pthread потоков
  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(pthreads[i], NULL) != 0) {
      fprintf(stderr, "pthread_join failed: %s\n", strerror(errno));
    }
  }

  // Ожидание завершения clone потоков
  for (int i = 0; i < NUM_THREADS; i++) {
    if (waitpid(clone_pids[i], NULL, 0) == -1) {
      fprintf(stderr, "waitpid failed: %s\n", strerror(errno));
    }
    free(stacks[i]);
  }

  printf("All threads exited\n");
  fflush(stdout); // Сбрасываем буфер перед завершением
  return EXIT_SUCCESS;
}
