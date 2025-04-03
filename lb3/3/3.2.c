#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Глобальные переменные для отслеживания состояния
volatile sig_atomic_t handler_active = 0;
volatile sig_atomic_t signals_received = 0;
volatile sig_atomic_t signals_processed = 0;
volatile sig_atomic_t immediate_signal_processed = 0;

// Прототипы функций
void signal_handler(int sig, siginfo_t *info, void *context);
void immediate_signal_handler(int sig, siginfo_t *info, void *context);
void setup_signal_handlers();
void block_signals(sigset_t *set, int include_realtime);
void unblock_signals(sigset_t *set);
void send_signals_to_self();
void print_pending_signals();
void test_signal_queuing();
void test_signal_priority();
void test_signal_blocking();

// Основной обработчик сигналов (для SIGUSR1, SIGUSR2, SIGRTMIN)
void signal_handler(int sig, siginfo_t *info, void *context) {
    signals_received++;
    int was_active = handler_active;
    handler_active = 1;
    
    printf("\n=== Обработчик сигнала %d (%s) начал работу ===\n", 
           sig, strsignal(sig));
    printf("Статистика:\n");
    printf("- Всего получено сигналов: %d\n", signals_received);
    printf("- Обработано сигналов: %d\n", signals_processed);
    printf("- Состояние: %s\n", was_active ? "вложенный вызов" : "первый вызов");fflush(stdout);
    
    // Имитация длительной обработки (3 секунды)
    printf("\nИмитация обработки (3 секунды):\n");fflush(stdout);
    for (int i = 0; i < 3; i++) {
        printf("  [%d] Работаю...\n", i+1);
        for (size_t j = 0; j < 100000000; j++); // Задержка
    }
    
    signals_processed++;
    handler_active = 0;
    printf("\n=== Обработчик сигнала %d завершил работу ===\n", sig);fflush(stdout);
}

// Обработчик для немедленного выполнения (SIGRTMIN+1)
void immediate_signal_handler(int sig, siginfo_t *info, void *context) {
    immediate_signal_processed++;
    printf("\n!!! Немедленный обработчик сигнала %d (%s) !!!\n", 
           sig, strsignal(sig));
    printf("Особенности:\n");
    printf("- Выполняется во время работы другого обработчика\n");
    printf("- Не блокирует другие сигналы (SA_NODEFER)\n");
    printf("- Всего немедленных обработок: %d\n", immediate_signal_processed);fflush(stdout);
}

// Настройка обработчиков сигналов
void setup_signal_handlers() {
    struct sigaction act;
    
    printf("\n=== Настройка обработчиков сигналов ===\n");
    
    // Обработчик для SIGUSR1 (блокирует SIGUSR2 и SIGRTMIN во время выполнения)
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR2);
    sigaddset(&act.sa_mask, SIGRTMIN);
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        perror("Ошибка sigaction(SIGUSR1)");
        exit(EXIT_FAILURE);
    }
    
    // Обработчик для SIGUSR2 (блокирует SIGUSR1 и SIGRTMIN во время выполнения)
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaddset(&act.sa_mask, SIGRTMIN);
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    
    if (sigaction(SIGUSR2, &act, NULL) < 0) {
        perror("Ошибка sigaction(SIGUSR2)");
        exit(EXIT_FAILURE);
    }
    
    // Обработчик для SIGRTMIN (обычный обработчик)
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaddset(&act.sa_mask, SIGUSR2);
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    
    if (sigaction(SIGRTMIN, &act, NULL) < 0) {
        perror("Ошибка sigaction(SIGRTMIN)");
        exit(EXIT_FAILURE);
    }
    
    // Обработчик для SIGRTMIN+1 (немедленный обработчик с SA_NODEFER)
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = immediate_signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_NODEFER; // Разрешаем повторные вызовы
    
    if (sigaction(SIGRTMIN+1, &act, NULL) < 0) {
        perror("Ошибка sigaction(SIGRTMIN+1)");
        exit(EXIT_FAILURE);
    }
    
    printf("\nНастроены обработчики:\n");
    printf("1. SIGUSR1:\n");
    printf("   - Блокирует SIGUSR2 и SIGRTMIN во время обработки\n");
    printf("2. SIGUSR2:\n");
    printf("   - Блокирует SIGUSR1 и SIGRTMIN во время обработки\n");
    printf("3. SIGRTMIN:\n");
    printf("   - Блокирует SIGUSR1 и SIGUSR2 во время обработки\n");
    printf("4. SIGRTMIN+1:\n");
    printf("   - Немедленный обработчик (SA_NODEFER)\n");
    printf("   - Не блокирует другие сигналы\n");fflush(stdout);
}

// Блокировка сигналов
void block_signals(sigset_t *set, int include_realtime) {
    sigemptyset(set);
    sigaddset(set, SIGUSR1);
    sigaddset(set, SIGUSR2);
    if (include_realtime) {
        sigaddset(set, SIGRTMIN);
        sigaddset(set, SIGRTMIN+1);
    }
    
    if (sigprocmask(SIG_BLOCK, set, NULL) < 0) {
        perror("Ошибка sigprocmask");
        exit(EXIT_FAILURE);
    }
    
    printf("\nБлокировка сигналов:\n");
    printf("- SIGUSR1\n- SIGUSR2\n");fflush(stdout);
    if (include_realtime) {
        printf("- SIGRTMIN\n- SIGRTMIN+1\n");fflush(stdout);
    }
}

// Разблокировка сигналов
void unblock_signals(sigset_t *set) {
    if (sigprocmask(SIG_UNBLOCK, set, NULL) < 0) {
        perror("Ошибка sigprocmask");
        exit(EXIT_FAILURE);
    }
    
    printf("\nРазблокировка сигналов\n");fflush(stdout);
}

// Печать ожидающих сигналов
void print_pending_signals() {
    sigset_t pending;
    sigpending(&pending);
    
    printf("\nТекущие ожидающие сигналы:\n");fflush(stdout);
    int has_pending = 0;
    
    if (sigismember(&pending, SIGUSR1)) {
        printf("- SIGUSR1\n");fflush(stdout);
        has_pending = 1;
    }
    if (sigismember(&pending, SIGUSR2)) {
        printf("- SIGUSR2\n");fflush(stdout);
        has_pending = 1;
    }
    if (sigismember(&pending, SIGRTMIN)) {
        printf("- SIGRTMIN\n");fflush(stdout);
        has_pending = 1;
    }
    if (sigismember(&pending, SIGRTMIN+1)) {
        printf("- SIGRTMIN+1\n");fflush(stdout);
        has_pending = 1;
    }
    
    if (!has_pending) {
        printf("Нет ожидающих сигналов\n");fflush(stdout);
    }
}

// Отправка сигналов самому себе
void send_signals_to_self() {
    printf("\n=== Тест 1: Отправка сигналов и их обработка ===\n");fflush(stdout);
    
    // Блокируем все сигналы перед отправкой
    sigset_t block_set;
    block_signals(&block_set, 1);
    
    // Отправляем несколько сигналов разных типов
    printf("\nПоследовательная отправка сигналов:\n");
    printf("1. Отправка 2 сигналов SIGUSR1...\n");fflush(stdout);
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR1);
    
    printf("2. Отправка 1 сигнала SIGUSR2...\n");fflush(stdout);
    kill(getpid(), SIGUSR2);
    
    printf("3. Отправка 1 сигнала SIGRTMIN...\n");fflush(stdout);
    kill(getpid(), SIGRTMIN);
    
    printf("4. Отправка 1 сигнала SIGRTMIN+1...\n");fflush(stdout);
    kill(getpid(), SIGRTMIN+1);
    
    printf("\nИтого отправлено:\n");
    printf("- 2 x SIGUSR1\n- 1 x SIGUSR2\n- 1 x SIGRTMIN\n- 1 x SIGRTMIN+1\n");fflush(stdout);
    
    // Показываем ожидающие сигналы
    print_pending_signals();
    
    // Разблокируем сигналы - обработчики начнут выполняться
    printf("\nРазблокируем сигналы для начала обработки:\n");fflush(stdout);
    unblock_signals(&block_set);
    
    // Во время работы обработчика отправляем еще сигналов
    if (handler_active) {
        printf("\nОтправка дополнительных сигналов во время обработки:\n");fflush(stdout);
        
        printf("1. Отправка SIGUSR2 (должен быть заблокирован)...\n");fflush(stdout);
        kill(getpid(), SIGUSR2);
        
        printf("2. Отправка SIGRTMIN+1 (должен выполниться немедленно)...\n");fflush(stdout);
        kill(getpid(), SIGRTMIN+1);
        
        print_pending_signals();
    }
}

// Тест очереди сигналов
void test_signal_queuing() {
    printf("\n=== Тест 2: Очередь сигналов ===\n");
    printf("Проверка накопления сигналов в очереди при блокировке\n");fflush(stdout);
    
    sigset_t block_set;
    block_signals(&block_set, 1);
    
    printf("\nОтправка множества сигналов при блокировке:\n");fflush(stdout);
    for (int i = 0; i < 5; i++) {
        printf("Отправка SIGUSR1 #%d...\n", i+1);fflush(stdout);
        kill(getpid(), SIGUSR1);
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Отправка SIGRTMIN+1 #%d...\n", i+1);fflush(stdout);
        kill(getpid(), SIGRTMIN+1);
    }
    
    print_pending_signals();
    
    printf("\nРазблокировка сигналов для обработки:\n");fflush(stdout);
    unblock_signals(&block_set);
    
    printf("\nОжидаемое поведение:\n");
    printf("- SIGUSR1 обработается только один раз (обычные сигналы не очередируются)\n");
    printf("- SIGRTMIN+1 обработается 3 раза (сигналы реального времени очередируются)\n");fflush(stdout);
}

// Тест приоритетов сигналов
void test_signal_priority() {
    printf("\n=== Тест 3: Приоритеты сигналов ===\n");
    printf("Проверка порядка обработки сигналов разного типа\n");fflush(stdout);
    
    sigset_t block_set;
    block_signals(&block_set, 1);
    
    printf("\nОтправка сигналов в обратном порядке приоритетов:\n");
    printf("1. Отправка SIGRTMIN+1 (низкий приоритет)...\n");fflush(stdout);
    kill(getpid(), SIGRTMIN+1);
    
    printf("2. Отправка SIGUSR1 (средний приоритет)...\n");fflush(stdout);
    kill(getpid(), SIGUSR1);
    
    printf("3. Отправка SIGRTMIN (высокий приоритет)...\n");fflush(stdout);
    kill(getpid(), SIGRTMIN);
    
    print_pending_signals();
    
    printf("\nРазблокировка сигналов для обработки:\n");fflush(stdout);
    unblock_signals(&block_set);
    
    printf("\nОжидаемое поведение:\n");
    printf("- Первым обработается SIGRTMIN (самый высокий приоритет)\n");
    printf("- Затем SIGUSR1\n");
    printf("- Последним SIGRTMIN+1\n");fflush(stdout);
}

// Тест блокировки сигналов
void test_signal_blocking() {
    printf("\n=== Тест 4: Блокировка сигналов ===\n");
    printf("Проверка маскирования сигналов во время обработки\n");fflush(stdout);
    
    printf("\nОтправка SIGUSR1 (блокирует SIGUSR2 и SIGRTMIN)...\n");fflush(stdout);
    kill(getpid(), SIGUSR1);
    
    printf("Отправка SIGUSR2 во время обработки SIGUSR1...\n");fflush(stdout);
    if (handler_active) {
        kill(getpid(), SIGUSR2);
        printf("Отправка SIGRTMIN во время обработки SIGUSR1...\n");fflush(stdout);
        kill(getpid(), SIGRTMIN);
        
        print_pending_signals();
        
        printf("\nОжидаемое поведение:\n");
        printf("- SIGUSR2 и SIGRTMIN будут заблокированы во время обработки SIGUSR1\n");
        printf("- Они обработаются только после завершения текущего обработчика\n"); fflush(stdout);
    }
}

int main() {
    printf("\n============================================\n");
    printf(" Демонстрация надежной обработки сигналов ");
    printf("\n============================================\n");
    printf("Цель: показать отложенную обработку сигналов\n");
    printf("      при поступлении во время выполнения обработчика\n\n");
    printf("PID процесса: %d\n", getpid());
    printf("Диапазон сигналов реального времени: %d-%d\n", SIGRTMIN, SIGRTMAX);
    fflush(stdout);
    // 1. Настройка обработчиков сигналов
    setup_signal_handlers();
    
    // 2. Основной тест отправки сигналов
    send_signals_to_self();
    
    // 3. Дополнительные тесты
    test_signal_queuing();
    test_signal_priority();
    test_signal_blocking();
    
    printf("\n============================================\n");
    printf(" Итоговые результаты \n");
    printf("============================================\n");
    printf("Всего получено сигналов: %d\n", signals_received);
    printf("Обычных сигналов обработано: %d\n", signals_processed);
    printf("Немедленных сигналов обработано: %d\n", immediate_signal_processed);
    
    printf("\nВыводы:\n");
    printf("1. Обычные сигналы (SIGUSR1, SIGUSR2) не накапливаются в очереди\n");
    printf("2. Сигналы реального времени (SIGRTMIN+) очередируются\n");
    printf("3. Сигналы блокируются во время работы обработчика (кроме SA_NODEFER)\n");
    printf("4. Сигналы с SA_NODEFER могут прерывать текущий обработчик\n");
    fflush(stdout);
    return 0;
}