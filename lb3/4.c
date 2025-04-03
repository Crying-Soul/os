#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define NUM_RTSIGS 32  // Используем 32 сигнала реального времени

// Глобальная переменная для экспериментов (можно считать счетчиком доставленных сигналов)
volatile sig_atomic_t delivered_count = 0;

// Обработчик сигналов с выводом информации
void sig_handler(int signo, siginfo_t *info, void *context) {
    (void)context;  // не используется
    delivered_count++;
    // Для сигналов реального времени выводим дополнительно значение, переданное через sigqueue
    if (signo >= SIGRTMIN && signo <= SIGRTMAX) {
        printf("[RT] Получен сигнал %d, sigval = %d, count = %d\n", signo, info->si_value.sival_int, delivered_count); fflush(stdout);
    } else {
        printf("[Norm] Получен сигнал %d, count = %d\n", signo, delivered_count); fflush(stdout);
    }
}

// Установка обработчика для заданного сигнала
void setup_handler(int signo) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sig_handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

// Эксперимент 4.1 и 4.2: Проверка приоритетов сигналов реального времени
void experiment_priorities() {
    
    printf("4.1 и 4.2: Приоритеты сигналов реального времени\n");
    printf("1. Проверка приоритетов SIGRTMIN и SIGRTMIN+1\n");
    printf("2. Проверка приоритетов для всего диапазона сигналов реального времени\n");
    printf("3. Определение сигналов с минимальным и максимальным приоритетом");

    fflush(stdout);

    /********************************************************************
     * Часть 1: Проверка стандартных сигналов (1-10)
     ********************************************************************/
    printf("\n=== Часть 1: Стандартные сигналы (1-10) ===\n");
    
    const int std_signals[] = {
        SIGHUP,    // 1
        SIGINT,    // 2
        SIGQUIT,   // 3
        SIGILL,    // 4
        SIGTRAP,   // 5
        SIGABRT,   // 6
        SIGBUS,    // 7
        SIGFPE,    // 8
        SIGKILL,   // 9
        SIGUSR1    // 10
    };
    const char* std_signal_names[] = {
        "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
        "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1"
    };
    const int num_std_signals = sizeof(std_signals)/sizeof(std_signals[0]);

    printf("Стандартные сигналы:\n");
    for (int i = 0; i < num_std_signals; i++) {
        printf("%2d) %-8s (%d)\n", i+1, std_signal_names[i], std_signals[i]);
    }
    printf("\n");

    // Устанавливаем обработчики (кроме SIGKILL)
    for (int i = 0; i < num_std_signals; i++) {
        if (std_signals[i] != SIGKILL) {
            setup_handler(std_signals[i]);
        }
    }

    // Блокируем сигналы (кроме SIGKILL)
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    for (int i = 0; i < num_std_signals; i++) {
        if (std_signals[i] != SIGKILL) {
            sigaddset(&block_set, std_signals[i]);
        }
    }
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    // Отправляем сигналы в обратном порядке (от 10 к 1)
    printf("Отправка сигналов в порядке от SIGUSR1 до SIGHUP:\n");
    for (int i = num_std_signals-1; i >= 0; i--) {
        if (std_signals[i] == SIGKILL) {
            printf("  Пропускаем SIGKILL (нельзя отправить самому себе)\n");
            continue;
        }
        
        if (kill(getpid(), std_signals[i]) == -1) {
            perror("kill");
        } else {
            printf("  Отправлен %-8s (%d)\n", std_signal_names[i], std_signals[i]);
        }
    }

    printf("\nРазблокируем сигналы...\n");
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);

    printf("\nРезультат для стандартных сигналов:\n");
    printf("  Обычно обрабатываются в порядке получения без строгих приоритетов\n");
    printf("  SIGKILL (%d) не может быть перехвачен или заблокирован\n\n", SIGKILL);

    /********************************************************************
     * Часть 2: Проверка приоритетов сигналов реального времени
     ********************************************************************/
    printf("=== Часть 2: Сигналы реального времени ===\n");
    printf("Справочная информация:\n");
    printf("  SIGRTMIN = %d\n", SIGRTMIN);
    printf("  SIGRTMAX = %d\n", SIGRTMAX);
    printf("  Всего сигналов реального времени: %d\n", SIGRTMAX - SIGRTMIN + 1);
    printf("  Приоритет: меньший номер => более высокий приоритет\n\n");

    // Подчасть 2.1: Проверка SIGRTMIN и SIGRTMIN+1
    printf("=== Подчасть 2.1: Проверка SIGRTMIN и SIGRTMIN+1 ===\n");
    
    setup_handler(SIGRTMIN);
    setup_handler(SIGRTMIN + 1);

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGRTMIN);
    sigaddset(&block_set, SIGRTMIN + 1);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    printf("Отправка сигналов в порядке: сначала SIGRTMIN+1, затем SIGRTMIN\n");
    union sigval sval;
    sval.sival_int = 100;
    if (sigqueue(getpid(), SIGRTMIN + 1, sval) == -1) {
        perror("sigqueue");
    } else {
        printf("  Отправлен SIGRTMIN+1 (%d) с значением %d\n", SIGRTMIN + 1, sval.sival_int);
    }
    sval.sival_int = 200;
    if (sigqueue(getpid(), SIGRTMIN, sval) == -1) {
        perror("sigqueue");
    } else {
        printf("  Отправлен SIGRTMIN (%d) с значением %d\n", SIGRTMIN, sval.sival_int);
    }

    printf("\nРазблокируем сигналы...\n");
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    
    printf("\nОжидаемый результат:\n");
    printf("  Должен первым обработаться SIGRTMIN, несмотря на то что он был отправлен вторым\n");

    // Подчасть 2.2: Проверка всего диапазона RT сигналов
    printf("\n=== Подчасть 2.2: Проверка всего диапазона RT сигналов ===\n");
    
    for (int sig = SIGRTMIN; sig <= SIGRTMAX; sig++) {
        setup_handler(sig);
    }

    sigemptyset(&block_set);
    for (int sig = SIGRTMIN; sig <= SIGRTMAX; sig++) {
        sigaddset(&block_set, sig);
    }
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    printf("Отправка сигналов в порядке от SIGRTMAX до SIGRTMIN:\n");
    for (int sig = SIGRTMAX; sig >= SIGRTMIN; sig--) {
        sval.sival_int = sig;
        if (sigqueue(getpid(), sig, sval) == -1) {
            perror("sigqueue");
        } else {
            printf("  Отправлен SIGRT(%d)\n", sig);
        }
    }

    printf("\nРазблокируем сигналы...\n");
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    
    printf("\nРезультат для RT сигналов:\n");
    printf("  Сигналы обрабатываются в порядке от SIGRTMIN до SIGRTMAX\n");
    printf("  Максимальный приоритет: SIGRTMIN (%d)\n", SIGRTMIN);
    printf("  Минимальный приоритет: SIGRTMAX (%d)\n", SIGRTMAX);
}


// Эксперимент 4.3.1: Очередь обычных сигналов
void experiment_normal_queue() {
    printf("Эксперимент 4.3.1: Очередь обычных сигналов (например, SIGUSR1)\n"); fflush(stdout);
    setup_handler(SIGUSR1);

    // Блокируем SIGUSR1
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Посылаем SIGUSR1 несколько раз
    for (int i = 0; i < 5; i++) {
        if (kill(getpid(), SIGUSR1) == -1) {
            perror("kill"); fflush(stdout);
        } else {
            printf("Отослан SIGUSR1 #%d\n", i+1); fflush(stdout);
        }
    }

    // Разблокируем SIGUSR1
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    printf("Обработчик вызвался %d раз (ожидалось 1, так как обычные сигналы не очередатся)\n", delivered_count); fflush(stdout);
}

// Эксперимент 4.3.2: Очередь сигналов реального времени
void experiment_realtime_queue() {
    printf("Эксперимент 4.3.2: Очередь сигналов реального времени\n"); fflush(stdout);
    // Выбираем один сигнал реального времени
    int rt_sig = SIGRTMIN + 2;
    setup_handler(rt_sig);

    // Блокируем выбранный сигнал
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, rt_sig);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Посылаем выбранный сигнал реального времени несколько раз с разными значениями
    union sigval sval;
    for (int i = 0; i < 5; i++) {
        sval.sival_int = i + 1;
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue"); fflush(stdout);
        } else {
            printf("Отослан SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int); fflush(stdout);
        }
    }
    // Разблокируем сигнал
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    printf("Обработчик вызвался %d раз (ожидалось 5, так как сигналы реального времени очередатся)\n", delivered_count); fflush(stdout);
}

// Эксперимент 4.3.3: Очередь смешанного набора сигналов (обычный + реального времени)
void experiment_mixed_queue() {
    printf("Эксперимент 4.3.3: Очередь смешанного набора сигналов\n"); fflush(stdout);
    // Используем SIGUSR2 как обычный и SIGRTMIN+3 как реального времени
    int rt_sig = SIGRTMIN + 3;
    setup_handler(SIGUSR2);
    setup_handler(rt_sig);

    // Блокируем оба сигнала
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR2);
    sigaddset(&block_set, rt_sig);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Отправляем сигналы: обычный, реального времени, обычный, реального времени
    if (kill(getpid(), SIGUSR2) == -1) {
        perror("kill SIGUSR2"); fflush(stdout);
    }
    union sigval sval;
    sval.sival_int = 101;
    if (sigqueue(getpid(), rt_sig, sval) == -1) {
        perror("sigqueue rt_sig"); fflush(stdout);
    }

    if (kill(getpid(), SIGUSR2) == -1) {
        perror("kill SIGUSR2"); fflush(stdout);
    }
    sval.sival_int = 202;
    if (sigqueue(getpid(), rt_sig, sval) == -1) {
        perror("sigqueue rt_sig"); fflush(stdout);
    }

    // Разблокируем сигналы
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }

    printf("Обработчик вызвался %d раз.\n", delivered_count); fflush(stdout);
    printf("Заметим, что обычные сигналы (SIGUSR2) не очередатся: они обрабатываются один раз, а сигналы реального времени – все.\n"); fflush(stdout);
}

// Эксперимент 4.4: Проверка FIFO для равноприоритетных сигналов реального времени
void experiment_fifo_order() {
    printf("Эксперимент 4.4: Проверка FIFO для равноприоритетных сигналов реального времени\n"); fflush(stdout);
    int rt_sig = SIGRTMIN + 4;
    setup_handler(rt_sig);

    // Блокируем сигнал
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, rt_sig);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Отправляем несколько сигналов одного типа с различными данными
    union sigval sval;
    const int count = 5;
    for (int i = 0; i < count; i++) {
        sval.sival_int = i + 1000;  // уникальное значение для идентификации порядка
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue"); fflush(stdout);
        } else {
            printf("Отослан SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int); fflush(stdout);
        }
    }
    // Разблокируем сигнал, ожидаем доставку и обработку
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask"); fflush(stdout);
        exit(EXIT_FAILURE);
    }

    printf("Если порядок доставки соответствует порядку отправки, то FIFO подтвержден.\n"); fflush(stdout);
}

int main(void) {
    printf("Программа экспериментов с сигналами.\n"); fflush(stdout);

    // Эксперимент 4.1 и 4.2: Приоритеты сигналов
    experiment_priorities();
    printf("Эксперимент 4.1 и 4.2 завершен.\n"); fflush(stdout);

    // Эксперимент 4.3.1: Очередь обычных сигналов
    experiment_normal_queue();
    printf("Эксперимент 4.3.1 завершен.\n"); fflush(stdout);

    // Эксперимент 4.3.2: Очередь сигналов реального времени
    experiment_realtime_queue();
    printf("Эксперимент 4.3.2 завершен.\n"); fflush(stdout);

    // Эксперимент 4.3.3: Смешанный набор сигналов
    experiment_mixed_queue();
    printf("Эксперимент 4.3.3 завершен.\n"); fflush(stdout);

    // Эксперимент 4.4: FIFO для сигналов реального времени
    experiment_fifo_order();
    printf("Эксперимент 4.4 завершен.\n"); fflush(stdout);

    return 0;
}