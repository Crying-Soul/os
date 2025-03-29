#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// Глобальные переменные для демонстрации
volatile sig_atomic_t flag = 0;
volatile sig_atomic_t usr1_count = 0;
volatile sig_atomic_t signal_received = 0;

// Функция для красивого вывода заголовка теста
void print_test_header(const char *test_name, const char *description) {
    printf("\n============================================================\n");
    printf("ТЕСТ: %s\n", test_name);
    printf("============================================================\n");
    printf("Описание: %s\n\n", description);
}

// Функция для ожидания сигнала
void wait_for_signal() {
    signal_received = 0;
    volatile int counter = 0;
    while (!signal_received && counter < 100000000) {
        counter++;
    }
}

// Функция для отправки сигнала самому себе
void send_self_signal(int sig) {
    printf("  Отправка сигнала %d текущему процессу...\n", sig);
    kill(getpid(), sig);
}

// Обработчик для SIGINT
void sigint_handler(int sig, siginfo_t *info, void *ucontext) {
    printf("\n  Получен SIGINT (сигнал %d) от процесса %d\n", 
           sig, info->si_pid);
    printf("  Код пользователя: %d\n", info->si_code);
    signal_received = 1;
}

// Обработчик для SIGTERM
void sigterm_handler(int sig) {
    printf("\n  Получен SIGTERM (сигнал %d)\n", sig);
    printf("  Этот обработчик использует старый стиль (без siginfo_t)\n");
    signal_received = 1;
}

// Обработчик для SIGUSR1
void sigusr1_handler(int sig, siginfo_t *info, void *ucontext) {
    usr1_count++;
    printf("\n  Получен SIGUSR1 (сигнал %d) от процесса %d\n", 
           sig, info->si_pid);
    printf("  Всего получено SIGUSR1: %d\n", usr1_count);
    
    if (info->si_code == SI_USER) {
        printf("  Сигнал отправлен пользователем (kill, raise)\n");
    } else if (info->si_code == SI_QUEUE) {
        printf("  Сигнал отправлен с данными (sigqueue)\n");
        printf("  Дополнительные данные: %d\n", info->si_value.sival_int);
    }
    signal_received = 1;
}

// Обработчик для SIGUSR2
void sigusr2_handler(int sig) {
    printf("\n  Получен SIGUSR2 (сигнал %d)\n", sig);
    printf("  Этот сигнал блокируется во время обработки SIGUSR1\n");
    signal_received = 1;
}

// Обработчик для SIGALRM
void sigalrm_handler(int sig) {
    printf("\n  Получен SIGALRM (сигнал %d)\n", sig);
    signal_received = 1;
}

// Обработчик для SIGCHLD
void sigchld_handler(int sig, siginfo_t *info, void *ucontext) {
    printf("\n  Получен SIGCHLD (сигнал %d)\n", sig);
    printf("  Дочерний процесс %d завершился с статусом %d\n", 
           info->si_pid, info->si_status);
    signal_received = 1;
}

// Обработчик для SIGPIPE
void sigpipe_handler(int sig) {
    printf("\n  Получен SIGPIPE (сигнал %d)\n", sig);
    printf("  Попытка записи в закрытый канал/сокет\n");
    signal_received = 1;
}

// Обработчик для SIGTSTP (Ctrl+Z)
void sigtstp_handler(int sig) {
    printf("\n  Получен SIGTSTP (сигнал %d)\n", sig);
    printf("  Процесс приостановлен. Для продолжения выполните 'fg'\n");
    signal_received = 1;
    // В автономном режиме просто продолжаем работу
    raise(SIGCONT);
}

// Функция для настройки обработчика сигнала
void setup_signal(int sig, void (*handler)(int, siginfo_t*, void*), 
                 int flags, sigset_t *mask, const char *desc) {
    struct sigaction act, oldact;
    
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    act.sa_flags = flags;
    
    if (mask) {
        act.sa_mask = *mask;
    } else {
        sigemptyset(&act.sa_mask);
    }
    
    if (sigaction(sig, &act, &oldact) < 0) {
        perror("  sigaction");
        exit(EXIT_FAILURE);
    }
    
    printf("  Настроен обработчик для %s (сигнал %d)\n", desc, sig);
    printf("  Флаги: 0x%x, Маска: %s\n", flags, mask ? "установлена" : "нет");
}

// Функция для создания дочернего процесса
void create_child_process() {
    pid_t pid = fork();
    if (pid == 0) {
        // Дочерний процесс
        printf("  Дочерний процесс %d запущен\n", getpid());
        volatile int counter = 0;
        while (counter < 50000000) counter++; // Задержка вместо sleep()
        printf("  Дочерний процесс %d завершается\n", getpid());
        exit(42);
    } else if (pid > 0) {
        // Родительский процесс
        volatile int counter = 0;
        while (counter < 100000000) counter++; // Задержка вместо sleep(2)
    } else {
        perror("  fork");
    }
}

int main() {
    printf("\n============================================================\n");
    printf("ПРОГРАММА ДЕМОНСТРАЦИИ РАБОТЫ С SIGACTION()\n");
    printf("============================================================\n");
    printf("PID: %d\n\n", getpid());
    
    // Тест 1: Обработка SIGINT с дополнительной информацией
    {
        print_test_header(
            "Обработка SIGINT (Ctrl+C) с SA_SIGINFO",
            "Демонстрация обработки SIGINT с использованием SA_SIGINFO для получения\n"
            "дополнительной информации о сигнале (pid отправителя, код и т.д.)"
        );
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGTERM);
        
        setup_signal(SIGINT, sigint_handler, 
                    SA_SIGINFO | SA_RESTART, &mask,
                    "SIGINT (Ctrl+C)");
        send_self_signal(SIGINT);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 2: Старый стиль обработчика (SIGTERM)
    {
        print_test_header(
            "Старый стиль обработчика (SIGTERM)",
            "Демонстрация простого обработчика сигналов без использования SA_SIGINFO"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigterm_handler;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGTERM, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  Настроен простой обработчик для SIGTERM (без SA_SIGINFO)\n");
        send_self_signal(SIGTERM);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 3: Обработка SIGUSR1 с блокировкой SIGUSR2
    {
        print_test_header(
            "Обработка SIGUSR1 с блокировкой SIGUSR2",
            "Демонстрация маскирования сигналов во время обработки другого сигнала"
        );
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR2);
        
        setup_signal(SIGUSR1, sigusr1_handler, 
                    SA_SIGINFO | SA_NODEFER, &mask,
                    "SIGUSR1 с блокировкой SIGUSR2");
        
        // Настроим также обработчик для SIGUSR2
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigusr2_handler;
        sigemptyset(&act.sa_mask);
        sigaction(SIGUSR2, &act, NULL);
        
        printf("  Отправка SIGUSR1 и сразу SIGUSR2...\n");
        send_self_signal(SIGUSR1);
        send_self_signal(SIGUSR2);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 4: Периодический SIGALRM
    {
        print_test_header(
            "Периодический SIGALRM",
            "Демонстрация работы с сигналами таймера"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigalrm_handler;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGALRM, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  Установлен обработчик для SIGALRM\n");
        printf("  Установка таймера на 1 секунду...\n");
        alarm(1);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 5: Обработка SIGCHLD
    {
        print_test_header(
            "Обработка завершения дочернего процесса (SIGCHLD)",
            "Демонстрация обработки сигналов от дочерних процессов"
        );
        setup_signal(SIGCHLD, sigchld_handler, 
                    SA_SIGINFO | SA_NOCLDSTOP, NULL,
                    "SIGCHLD с информацией о статусе");
        
        create_child_process();
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 6: Игнорирование SIGHUP
    {
        print_test_header(
            "Игнорирование SIGHUP",
            "Демонстрация игнорирования сигналов"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = SIG_IGN;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGHUP, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  SIGHUP теперь игнорируется\n");
        send_self_signal(SIGHUP);
        volatile int counter = 0;
        while (counter < 50000000) counter++; // Задержка вместо sleep()
        printf("  SIGHUP был проигнорирован\n");
        printf("\n  Тест завершен\n");
    }
    
    // Тест 7: Восстановление обработчика по умолчанию для SIGQUIT
    {
        print_test_header(
            "Восстановление обработчика по умолчанию (SIGQUIT)",
            "Демонстрация восстановления стандартного поведения сигнала"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = SIG_DFL;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGQUIT, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  Для SIGQUIT восстановлен обработчик по умолчанию\n");
        printf("  Внимание: Отправка SIGQUIT теперь завершит программу!\n");
        printf("  (Автоматическая отправка SIGQUIT отключена в целях безопасности)\n");
        volatile int counter = 0;
        while (counter < 50000000) counter++; // Задержка
        printf("\n  Тест завершен\n");
    }
    
    // Тест 8: Обработка SIGPIPE
    {
        print_test_header(
            "Обработка SIGPIPE",
            "Демонстрация обработки сигнала при записи в закрытый канал"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigpipe_handler;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGPIPE, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  Настроен обработчик для SIGPIPE\n");
        send_self_signal(SIGPIPE);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 9: Обработка SIGTSTP (Ctrl+Z)
    {
        print_test_header(
            "Обработка SIGTSTP (Ctrl+Z)",
            "Демонстрация обработки сигнала приостановки процесса"
        );
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigtstp_handler;
        sigemptyset(&act.sa_mask);
        
        if (sigaction(SIGTSTP, &act, NULL) < 0) {
            perror("  sigaction");
            exit(EXIT_FAILURE);
        }
        printf("  Настроен обработчик для SIGTSTP (Ctrl+Z)\n");
        printf("  Имитация получения SIGTSTP...\n");
        send_self_signal(SIGTSTP);
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    // Тест 10: Маскирование сигналов
    {
        print_test_header(
            "Маскирование сигналов в критической секции",
            "Демонстрация блокировки сигналов во время выполнения важного кода"
        );
        sigset_t newmask, oldmask;
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGINT);
        sigaddset(&newmask, SIGTERM);
        
        printf("  Вход в критическую секцию (блокировка SIGINT и SIGTERM)\n");
        if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
            perror("  sigprocmask");
            exit(EXIT_FAILURE);
        }
        
        printf("  Критическая секция - SIGINT и SIGTERM заблокированы\n");
        printf("  Отправка SIGINT... (должен быть отложен)\n");
        send_self_signal(SIGINT);
        
        volatile int counter = 0;
        while (counter < 50000000) counter++; // Задержка
        
        printf("  Выход из критической секции (разблокировка сигналов)\n");
        if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
            perror("  sigprocmask");
            exit(EXIT_FAILURE);
        }
        printf("  Сигналы разблокированы\n");
        wait_for_signal(); // Ожидаем обработку отложенного сигнала
        printf("\n  Тест завершен\n");
    }
    
    // Тест 11: Очередь сигналов с данными (sigqueue)
    {
        print_test_header(
            "Очередь сигналов с данными (sigqueue)",
            "Демонстрация отправки сигналов с дополнительными данными"
        );
        union sigval value;
        value.sival_int = 12345;
        
        printf("  Отправка SIGUSR1 с дополнительными данными...\n");
        if (sigqueue(getpid(), SIGUSR1, value) < 0) {
            perror("  sigqueue");
        }
        wait_for_signal();
        printf("\n  Тест завершен\n");
    }
    
    printf("\n============================================================\n");
    printf("ВСЕ ТЕСТЫ УСПЕШНО ЗАВЕРШЕНЫ\n");
    printf("============================================================\n");
    return 0;
}