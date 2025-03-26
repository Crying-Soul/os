#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Функция, создающая нагрузку (имитация длительной работы)
void worker(int seconds) {
    volatile long long sum = 0;
    time_t start = time(NULL);
    while (time(NULL) - start < seconds) {
        for (long long i = 0; i < 1000000; i++) {
            sum += i;
        }
    }
}

// Расширенный обработчик для сигналов, использующий siginfo_t
void handler(int signo, siginfo_t *info, void *context) {
    (void)context; // Не используем, чтобы избежать предупреждений
    if (signo == SIGUSR1) {
        printf("Обработчик SIGUSR1: получен сигнал %d от процесса %d\n", signo, info->si_pid);
        // Имитация длительной обработки
        worker(3);
        printf("Обработчик SIGUSR1: завершена обработка сигнала %d\n", signo);
    } else if (signo == SIGUSR2) {
        printf("Обработчик SIGUSR2: получен сигнал %d от процесса %d\n", signo, info->si_pid);
        worker(1);
        printf("Обработчик SIGUSR2: завершена обработка сигнала %d\n", signo);
    }
}

int main(void) {
    struct sigaction sa, old_sa_usr1, old_sa_usr2;

    // Обнуляем структуру для корректной инициализации
    memset(&sa, 0, sizeof(sa));
    // Устанавливаем расширенный обработчик
    sa.sa_sigaction = handler;
    // Блокировка SIGUSR2 во время обработки SIGUSR1
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR2);
    // Используем расширенный формат обработчика
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    // Установка обработчика для SIGUSR1
    if (sigaction(SIGUSR1, &sa, &old_sa_usr1) == -1) {
        perror("Ошибка установки обработчика SIGUSR1");
        exit(EXIT_FAILURE);
    }

    // Для SIGUSR2 – блокировка сигналов не нужна (можно оставить пустую маску)
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGUSR2, &sa, &old_sa_usr2) == -1) {
        perror("Ошибка установки обработчика SIGUSR2");
        exit(EXIT_FAILURE);
    }

    // Генерация сигналов: сначала SIGUSR1
    printf("Отправляем SIGUSR1\n");
    kill(getpid(), SIGUSR1);

    // Небольшая задержка для имитации работы (до поступления следующего сигнала)
    worker(1);

    printf("Отправляем SIGUSR2 (во время обработки SIGUSR1 сигнал будет отложен)\n");
    kill(getpid(), SIGUSR2);

    // Отправляем ещё один SIGUSR2
    worker(1);
    printf("Отправляем еще один SIGUSR2\n");
    kill(getpid(), SIGUSR2);

    // Ждем завершения всех обработок
    worker(5);

    // Восстанавливаем исходные обработчики
    if (sigaction(SIGUSR1, &old_sa_usr1, NULL) == -1) {
        perror("Ошибка восстановления обработчика SIGUSR1");
    }
    if (sigaction(SIGUSR2, &old_sa_usr2, NULL) == -1) {
        perror("Ошибка восстановления обработчика SIGUSR2");
    }
    printf("Исходные обработчики восстановлены\n");

    return 0;
}
