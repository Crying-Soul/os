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
        printf("  [RT] Получен сигнал %d, sigval = %d, count = %d\n", signo, info->si_value.sival_int, delivered_count);
    } else {
        printf("  [Norm] Получен сигнал %d, count = %d\n", signo, delivered_count);
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

// Вывод заголовка эксперимента
void print_experiment_header(const char *title, const char *description) {
    printf("\n================================================================\n");
    printf("ЭКСПЕРИМЕНТ: %s\n", title);
    printf("================================================================\n");
    printf("Описание: %s\n\n", description);
}

// Эксперимент 4.1 и 4.2: Проверка приоритетов сигналов реального времени
void experiment_priorities() {
    print_experiment_header(
        "4.1 и 4.2: Приоритеты сигналов реального времени",
        "Проверка, что сигналы с меньшими номерами имеют более высокий приоритет.\n"
        "Отправляем SIGRTMIN+1, затем SIGRTMIN - должен обработаться первым SIGRTMIN."
    );
    
    printf("Справочная информация:\n");
    printf("  SIGRTMIN = %d\n", SIGRTMIN);
    printf("  SIGRTMAX = %d\n", SIGRTMAX);
    printf("  Приоритет: меньший номер => более высокий приоритет\n\n");
    
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

    printf("Отправка сигналов в порядке: сначала SIGRTMIN+1, затем SIGRTMIN\n");
    // Посылаем сигналы в обратном порядке (сначала более высокий номер, затем меньший)
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

    printf("\nРазблокируем сигналы и ожидаем обработки...\n");
    // Разблокируем сигналы и посмотрим порядок доставки
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем немного, чтобы обработчики вывели сообщение
    sleep(1);
    
    printf("\nОжидаемый результат:\n");
    printf("  Должен первым обработаться SIGRTMIN, несмотря на то что он был отправлен вторым\n");
}

// Эксперимент 4.3.1: Очередь обычных сигналов
void experiment_normal_queue() {
    print_experiment_header(
        "4.3.1: Очередь обычных сигналов (SIGUSR1)",
        "Проверка, что несколько отправленных обычных сигналов не накапливаются в очереди,\n"
        "а обрабатывается только один экземпляр."
    );
    
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

    printf("Отправляем SIGUSR1 5 раз подряд...\n");
    // Посылаем SIGUSR1 несколько раз
    for (int i = 0; i < 5; i++) {
        if (kill(getpid(), SIGUSR1) == -1) {
            perror("kill");
        } else {
            printf("  Отправлен SIGUSR1 #%d\n", i+1);
        }
    }

    printf("\nРазблокируем SIGUSR1...\n");
    // Разблокируем SIGUSR1
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем, чтобы обработчик сработал
    sleep(1);
    
    printf("\nРезультат:\n");
    printf("  Обработчик вызвался %d раз\n", delivered_count);
    printf("Ожидаемое поведение: 1 раз, так как обычные сигналы не очередируются\n");
}

// Эксперимент 4.3.2: Очередь сигналов реального времени
void experiment_realtime_queue() {
    print_experiment_header(
        "4.3.2: Очередь сигналов реального времени",
        "Проверка, что сигналы реального времени накапливаются в очереди\n"
        "и обрабатываются все отправленные экземпляры."
    );
    
    // Выбираем один сигнал реального времени
    int rt_sig = SIGRTMIN + 2;
    printf("Используем сигнал реального времени: %d (SIGRTMIN+2)\n\n", rt_sig);
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

    printf("Отправляем сигнал %d 5 раз с разными значениями...\n", rt_sig);
    // Посылаем выбранный сигнал реального времени несколько раз с разными значениями
    union sigval sval;
    for (int i = 0; i < 5; i++) {
        sval.sival_int = i + 1;
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue");
        } else {
            printf("  Отправлен SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int);
        }
    }
    
    printf("\nРазблокируем сигнал...\n");
    // Разблокируем сигнал
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    // Подождем для обработки
    sleep(1);
    
    printf("\nРезультат:\n");
    printf("  Обработчик вызвался %d раз\n", delivered_count);
    printf("Ожидаемое поведение: 5 раз, так как сигналы реального времени очередируются\n");
}

// Эксперимент 4.3.3: Очередь смешанного набора сигналов (обычный + реального времени)
void experiment_mixed_queue() {
    print_experiment_header(
        "4.3.3: Смешанный набор сигналов (обычный + реального времени)",
        "Проверка обработки при одновременной отправке обычных сигналов и сигналов\n"
        "реального времени. Обычные сигналы не должны накапливаться в очереди."
    );
    
    // Используем SIGUSR2 как обычный и SIGRTMIN+3 как реального времени
    int rt_sig = SIGRTMIN + 3;
    printf("Используемые сигналы:\n");
    printf("  Обычный: SIGUSR2 (%d)\n", SIGUSR2);
    printf("  Реального времени: %d (SIGRTMIN+3)\n\n", rt_sig);
    
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

    printf("Отправляем сигналы в последовательности:\n");
    printf("  1. Обычный SIGUSR2\n");
    printf("  2. Реального времени %d\n", rt_sig);
    printf("  3. Обычный SIGUSR2\n");
    printf("  4. Реального времени %d\n\n", rt_sig);
    
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

    printf("Разблокируем сигналы...\n");
    // Разблокируем сигналы
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    
    printf("\nРезультат:\n");
    printf("  Обработчик вызвался %d раз.\n", delivered_count);
    printf("Ожидаемое поведение:\n");
    printf("  - SIGUSR2 обработается только 1 раз (последний)\n");
    printf("  - Оба SIGRT обработаются\n");
}

// Эксперимент 4.4: Проверка FIFO для равноприоритетных сигналов реального времени
void experiment_fifo_order() {
    print_experiment_header(
        "4.4: FIFO для равноприоритетных сигналов реального времени",
        "Проверка, что сигналы реального времени с одинаковым приоритетом\n"
        "обрабатываются в порядке FIFO (первым пришел - первым обработан)."
    );
    
    int rt_sig = SIGRTMIN + 4;
    printf("Используем сигнал реального времени: %d (SIGRTMIN+4)\n\n", rt_sig);
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

    printf("Отправляем 5 сигналов %d с последовательными значениями...\n", rt_sig);
    // Отправляем несколько сигналов одного типа с различными данными
    union sigval sval;
    const int count = 5;
    for (int i = 0; i < count; i++) {
        sval.sival_int = i + 1000;  // уникальное значение для идентификации порядка
        if (sigqueue(getpid(), rt_sig, sval) == -1) {
            perror("sigqueue");
        } else {
            printf("  Отправлен SIGRT(%d) с данными %d\n", rt_sig, sval.sival_int);
        }
    }
    
    printf("\nРазблокируем сигнал...\n");
    // Разблокируем сигнал, ожидаем доставку и обработку
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    
    printf("\nОжидаемый результат:\n");
    printf("  Сигналы должны обработаться в порядке отправки (значения 1000, 1001, 1002, 1003, 1004)\n");
}

int main(void) {
    printf("\n================================================================");
    printf("\nНАЧАЛО ПРОГРАММЫ ДЛЯ ЭКСПЕРИМЕНТОВ С СИГНАЛАМИ");
    printf("\n================================================================");
    printf("\nКаждый эксперимент будет выполняться последовательно с подробным\n"
           "описанием действий и ожидаемых результатов.\n");

    // Эксперимент 4.1 и 4.2: Приоритеты сигналов
    experiment_priorities();
    printf("\n=== ЭКСПЕРИМЕНТ 4.1 и 4.2 ЗАВЕРШЕН ===\n");

    // Эксперимент 4.3.1: Очередь обычных сигналов
    experiment_normal_queue();
    printf("\n=== ЭКСПЕРИМЕНТ 4.3.1 ЗАВЕРШЕН ===\n");

    // Эксперимент 4.3.2: Очередь сигналов реального времени
    experiment_realtime_queue();
    printf("\n=== ЭКСПЕРИМЕНТ 4.3.2 ЗАВЕРШЕН ===\n");

    // Эксперимент 4.3.3: Смешанный набор сигналов
    experiment_mixed_queue();
    printf("\n=== ЭКСПЕРИМЕНТ 4.3.3 ЗАВЕРШЕН ===\n");

    // Эксперимент 4.4: FIFO для сигналов реального времени
    experiment_fifo_order();
    printf("\n=== ЭКСПЕРИМЕНТ 4.4 ЗАВЕРШЕН ===\n");

    printf("\n================================================================");
    printf("\nВСЕ ЭКСПЕРИМЕНТЫ УСПЕШНО ЗАВЕРШЕНЫ");
    printf("\n================================================================");
   
    return 0;
}