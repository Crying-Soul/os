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
        printf("[RT] Получен сигнал %d, sigval = %d, count = %d\n", signo, info->si_value.sival_int, delivered_count);
    } else {
        printf("[Norm] Получен сигнал %d, count = %d\n", signo, delivered_count);
    }
}

// Установка обработчика для заданного сигнала
void setup_handler(int signo) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sig_handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

// Эксперимент 4.1 и 4.2: Проверка приоритетов сигналов реального времени
void experiment_priorities() {
    printf("Эксперимент 4.1 и 4.2: Приоритеты сигналов\n");
    printf("SIGRTMIN = %d\n", SIGRTMIN);
    printf("SIGRTMAX = %d\n", SIGRTMAX);
    printf("Приоритет: меньший номер => более высокий приоритет.\n");
    
    // Устанавливаем обработчики для двух сигналов реального времени: SIGRTMIN и SIGRTMIN+1
    setup_handler(SIGRTMIN);
    setup_handler(SIGRTMIN + 1);

    // Блокируем оба сигнала
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGRTMIN);
    sigaddset(&block_set, SIGRTMIN + 1);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    // Посылаем сигналы в обратном порядке (сначала более высокий номер, затем меньший)
    union sigval sval;
    sval.sival_int = 100;
    if (sigqueue(getpid(), SIGRTMIN + 1, sval) == -1) {
        perror("sigqueue");
    }
    sval.sival_int = 200;
    if (sigqueue(getpid(), SIGRTMIN, sval) == -1) {
        perror("sigqueue");
    }

    // Разблокируем сигналы и посмотрим порядок доставки
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем немного, чтобы обработчики вывели сообщение
    sleep(1);
}

// Эксперимент 4.3.1: Очередь обычных сигналов
void experiment_normal_queue() {
    printf("Эксперимент 4.3.1: Очередь обычных сигналов (например, SIGUSR1)\n");
    setup_handler(SIGUSR1);

    // Блокируем SIGUSR1
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Посылаем SIGUSR1 несколько раз
    for (int i = 0; i < 5; i++) {
        if (kill(getpid(), SIGUSR1) == -1) {
            perror("kill");
        } else {
            printf("Отослан SIGUSR1 #%d\n", i+1);
        }
    }

    // Разблокируем SIGUSR1
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем, чтобы обработчик сработал
    sleep(1);
    printf("Обработчик вызвался %d раз (ожидалось 1, так как обычные сигналы не очередатся)\n", delivered_count);
}

// Эксперимент 4.3.2: Очередь сигналов реального времени
void experiment_realtime_queue() {
    printf("Эксперимент 4.3.2: Очередь сигналов реального времени\n");
    // Выбираем один сигнал реального времени
    int rt_sig = SIGRTMIN + 2;
    setup_handler(rt_sig);

    // Блокируем выбранный сигнал
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, rt_sig);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Посылаем выбранный сигнал реального времени несколько раз с разными значениями
    union sigval sval;
    for (int i = 0; i < 5; i++) {
        sval.sival_int = i + 1;
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue");
        } else {
            printf("Отослан SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int);
        }
    }
    // Разблокируем сигнал
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем для обработки
    sleep(1);
    printf("Обработчик вызвался %d раз (ожидалось 5, так как сигналы реального времени очередатся)\n", delivered_count);
}

// Эксперимент 4.3.3: Очередь смешанного набора сигналов (обычный + реального времени)
void experiment_mixed_queue() {
    printf("Эксперимент 4.3.3: Очередь смешанного набора сигналов\n");
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
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Отправляем сигналы: обычный, реального времени, обычный, реального времени
    if (kill(getpid(), SIGUSR2) == -1)
        perror("kill SIGUSR2");
    union sigval sval;
    sval.sival_int = 101;
    if (sigqueue(getpid(), rt_sig, sval) == -1)
        perror("sigqueue rt_sig");

    if (kill(getpid(), SIGUSR2) == -1)
        perror("kill SIGUSR2");
    sval.sival_int = 202;
    if (sigqueue(getpid(), rt_sig, sval) == -1)
        perror("sigqueue rt_sig");

    // Разблокируем сигналы
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    printf("Обработчик вызвался %d раз.\n", delivered_count);
    printf("Заметим, что обычные сигналы (SIGUSR2) не очередатся: они обрабатываются один раз, а сигналы реального времени – все.\n");
}

// Эксперимент 4.4: Проверка FIFO для равноприоритетных сигналов реального времени
void experiment_fifo_order() {
    printf("Эксперимент 4.4: Проверка FIFO для равноприоритетных сигналов реального времени\n");
    int rt_sig = SIGRTMIN + 4;
    setup_handler(rt_sig);

    // Блокируем сигнал
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, rt_sig);
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Отправляем несколько сигналов одного типа с различными данными
    union sigval sval;
    const int count = 5;
    for (int i = 0; i < count; i++) {
        sval.sival_int = i + 1000;  // уникальное значение для идентификации порядка
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue");
        } else {
            printf("Отослан SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int);
        }
    }
    // Разблокируем сигнал, ожидаем доставку и обработку
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    printf("Если порядок доставки соответствует порядку отправки, то FIFO подтвержден.\n");
}

int main(void) {
    printf("Программа экспериментов с сигналами.\n");

    // Эксперимент 4.1 и 4.2: Приоритеты сигналов
    experiment_priorities();
    printf("Эксперимент 4.1 и 4.2 завершен.\n");

    // Эксперимент 4.3.1: Очередь обычных сигналов
    experiment_normal_queue();
    printf("Эксперимент 4.3.1 завершен.\n");

    // Эксперимент 4.3.2: Очередь сигналов реального времени
    experiment_realtime_queue();
    printf("Эксперимент 4.3.2 завершен.\n");

    // Эксперимент 4.3.3: Смешанный набор сигналов
    experiment_mixed_queue();
    printf("Эксперимент 4.3.3 завершен.\n");

    // Эксперимент 4.4: FIFO для сигналов реального времени
    experiment_fifo_order();
    printf("Эксперимент 4.4 завершен.\n");

   
    return 0;
}
