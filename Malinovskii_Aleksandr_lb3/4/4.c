
/******************************************************************************
 * 
 *  Цель:
 *      Предоставить исчерпывающее исследование методов обработки сигналов
 *      в C с использованием функции `sigaction`. Продемонстрировать различные
 *      сценарии, от базового перехвата сигналов до продвинутых функций, таких
 *      как маскирование сигналов, сигналы на основе таймера и межпроцессная
 *      обработка сигналов.
 *
 *  Описание:
 *      Код структурирован как серия отдельных тестов, каждый из которых
 *      предназначен для выделения конкретного аспекта управления сигналами.
 *      Эти тесты охватывают:
 *
 *          - **Базовая обработка сигналов**: демонстрирует перехват `SIGINT`
 *            (Ctrl+C) и отображение информации о сигнале.
 *          - **Традиционная обработка сигналов**: иллюстрирует использование
 *            обработчиков сигналов старого стиля с `SIGTERM`.
 *          - **Маскирование сигналов**: показывает, как блокировать `SIGUSR2` во
 *            время обработки `SIGUSR1`.
 *          - **Сигналы на основе таймера**: исследует использование `SIGALRM` для
 *            запуска событий через регулярные интервалы.
 *          - **Межпроцессное взаимодействие (сигналы)**: демонстрирует, как
 *            родительский процесс может обрабатывать `SIGCHLD`, когда дочерний
 *            процесс завершается.
 *          - **Игнорирование сигналов**: объясняет, как игнорировать сигналы, такие
 *            как `SIGHUP`.
 *          - **Восстановление обработчиков по умолчанию**: показывает, как
 *            вернуться к обработке сигналов по умолчанию для таких сигналов, как
 *            `SIGQUIT`.
 *          - **Обработка `SIGPIPE`**: демонстрирует обработку `SIGPIPE` при записи
 *            в закрытый канал.
 *          - **Приостановка процесса (`SIGTSTP`)**: иллюстрирует, как обрабатывать
 *            `SIGTSTP` (Ctrl+Z) для приостановки процесса.
 *          - **Критические секции и маскирование сигналов**: показывает, как
 *            блокировать сигналы во время критических секций кода, чтобы
 *            предотвратить состояния гонки.
 *          - **`sigqueue` и очередь сигналов**: демонстрирует, как отправлять
 *            сигналы со связанными данными с использованием `sigqueue`.
 *
 *  Ключевые концепции:
 *      - **`sigaction`**: Основная функция, используемая для настройки
 *        обработчиков сигналов, предлагающая больший контроль и гибкость по
 *        сравнению с традиционной функцией `signal`.
 *      - **`SA_SIGINFO`**: Флаг, который позволяет обработчику сигналов получать
 *        расширенную информацию о сигнале (например, PID отправителя, код
 *        сигнала).
 *      - **Наборы сигналов (`sigset_t`)**: Структуры данных, используемые для
 *        управления наборами сигналов для целей маскирования.
 *      - **Маскирование сигналов (`sigprocmask`)**: Механизм временной блокировки
 *        определенных сигналов для обеспечения атомарных операций.
 *      - **`sigqueue`**: Функция, позволяющая отправлять сигналы со связанными
 *        данными.
 *
 *  Примечания:
 *      - Код включает базовую проверку ошибок, но для производственной среды
 *        может потребоваться более надежная обработка ошибок.
 *      - Циклы занятого ожидания используются в демонстрационных целях, но их
 *        следует заменить более эффективными механизмами, такими как `nanosleep` или
 *        условные переменные в реальных приложениях.
 *
 ******************************************************************************/

 #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

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
// Эксперимент 4.1 и 4.2: Проверка приоритетов сигналов реального времени
// Эксперимент 4.1 и 4.2: Проверка приоритетов стандартных сигналов
// Эксперимент 4.1 и 4.2: Проверка приоритетов сигналов
void experiment_priorities() {
    print_experiment_header(
        "4.1 и 4.2: Приоритеты сигналов",
        "1. Проверка стандартных сигналов (1-10)\n"
        "2. Проверка приоритетов SIGRTMIN и SIGRTMIN+1\n"
        "3. Проверка приоритетов для всего диапазона сигналов реального времени\n"
        "4. Определение сигналов с минимальным и максимальным приоритетом"
    );

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
    printf("Эксперимент 4.3.3 (расширенный v2): Очередь смешанного набора сигналов с чередованием\n"); 
    fflush(stdout);
    
    // Используем SIGUSR1, SIGUSR2, SIGURG, SIGIO как обычные сигналы
    // и SIGRTMIN+1, SIGRTMIN+2, SIGRTMIN+3, SIGRTMIN+4 как реального времени
    int rt_sigs[] = {SIGRTMIN+1, SIGRTMIN+2, SIGRTMIN+3, SIGRTMIN+4};
    int std_sigs[] = {SIGUSR2, SIGURG, SIGIO, SIGUSR1,};
    
    // Настраиваем обработчики для всех сигналов
    for (int i = 0; i < sizeof(std_sigs)/sizeof(std_sigs[0]); i++) {
        setup_handler(std_sigs[i]);
    }
    for (int i = 0; i < sizeof(rt_sigs)/sizeof(rt_sigs[0]); i++) {
        setup_handler(rt_sigs[i]);
    }

    // Блокируем все сигналы
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    for (int i = 0; i < sizeof(std_sigs)/sizeof(std_sigs[0]); i++) {
        sigaddset(&block_set, std_sigs[i]);
    }
    for (int i = 0; i < sizeof(rt_sigs)/sizeof(rt_sigs[0]); i++) {
        sigaddset(&block_set, rt_sigs[i]);
    }
    
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask"); 
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    delivered_count = 0;

    // Отправляем расширенную серию сигналов, начиная с RT и чередуя
    printf("Отправляем последовательность сигналов (начинаем с RT):\n");
    printf("SIGRTMIN+1, SIGUSR1, SIGRTMIN+2, SIGUSR2, SIGRTMIN+3, SIGURG, SIGRTMIN+4, SIGIO\n");
    fflush(stdout);
    
     // 4. Обычный SIGUSR2
     if (kill(getpid(), SIGUSR2) == -1) {
        perror("kill SIGUSR2"); 
        fflush(stdout);
    }

    // 1. RT с данными 1001 (первый сигнал)
    union sigval sval;
    sval.sival_int = 1001;
    if (sigqueue(getpid(), rt_sigs[0], sval) == -1) {
        perror("sigqueue rt_sigs[0]"); 
        fflush(stdout);
    }
    
    // 2. Обычный SIGUSR1
    if (kill(getpid(), SIGUSR1) == -1) {
        perror("kill SIGUSR1"); 
        fflush(stdout);
    }
    
    // 3. RT с данными 1002
    sval.sival_int = 1002;
    if (sigqueue(getpid(), rt_sigs[1], sval) == -1) {
        perror("sigqueue rt_sigs[1]"); 
        fflush(stdout);
    }
    
    
    // 5. RT с данными 1003
    sval.sival_int = 1003;
    if (sigqueue(getpid(), rt_sigs[2], sval) == -1) {
        perror("sigqueue rt_sigs[2]"); 
        fflush(stdout);
    }
    
    // 6. Обычный SIGURG
    if (kill(getpid(), SIGURG) == -1) {
        perror("kill SIGURG"); 
        fflush(stdout);
    }
    
    // 7. RT с данными 1004
    sval.sival_int = 1004;
    if (sigqueue(getpid(), rt_sigs[3], sval) == -1) {
        perror("sigqueue rt_sigs[3]"); 
        fflush(stdout);
    }
    
    // 8. Обычный SIGIO
    if (kill(getpid(), SIGIO) == -1) {
        perror("kill SIGIO"); 
        fflush(stdout);
    }

    // Разблокируем сигналы
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        perror("sigprocmask"); 
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    printf("Обработчик вызвался %d раз.\n", delivered_count); 
    fflush(stdout);
    
    printf("Ожидаемое поведение:\n");
    printf("1. Обычные сигналы (SIGUSR1, SIGUSR2, SIGURG, SIGIO) не очередиваются - обработчик вызовется только по одному разу для каждого типа\n");
    printf("2. Все сигналы реального времени (4 разных) будут обработаны с сохранением порядка отправки\n");
    printf("3. Данные (sival_int) для RT сигналов должны быть сохранены в порядке 1001, 1002, 1003, 1004\n");
    printf("4. Первым должен быть обработан RT сигнал (SIGRTMIN+1), затем обычный (SIGUSR1) и т.д.\n");
    fflush(stdout);
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