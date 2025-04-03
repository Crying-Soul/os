#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

/* Константы для настройки работы программы */
#define MAX_MSG_SIZE 256                // Максимальный размер текста сообщения
#define MSG_TYPE_LOW 1                  // Тип сообщения: низкий приоритет
#define MSG_TYPE_MED 2                  // Тип сообщения: средний приоритет
#define MSG_TYPE_HIGH 3                 // Тип сообщения: высокий приоритет
#define MSG_TYPE_CHILD 4                // Тип сообщения от дочернего процесса
#define MSG_TYPE_PRIORITY_HIGH 10       // Высокий приоритет для эксперимента
#define MSG_TYPE_PRIORITY_MED 5         // Средний приоритет для эксперимента
#define MSG_TYPE_PRIORITY_LOW 1         // Низкий приоритет для эксперимента
#define TOTAL_MESSAGES 8                // Общее количество тестовых сообщений
#define QUEUE_PERMISSIONS 0666          // Права доступа к очереди
#define PERFORMANCE_TEST_MSGS 100      // Количество сообщений для теста производительности

/* Структура сообщения с дополнительными полями */
typedef struct {
    long msg_type;                      // Тип сообщения
    char msg_text[MAX_MSG_SIZE];        // Текст сообщения
    struct timeval send_time;           // Время отправки
    pid_t sender_pid;                   // PID отправителя
} Message;

/* Функция для печати времени с миллисекундами */
void print_time_with_ms(const struct timeval *tv) {
    if (tv->tv_sec == 0 && tv->tv_usec == 0) {
        printf("никогда");
        return;
    }
    
    char buffer[80];
    struct tm *timeinfo = localtime(&tv->tv_sec);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s.%03ld", buffer, tv->tv_usec / 1000);
}

/* Вывод информации об очереди сообщений */
void print_queue_info(int msgid) {
    struct msqid_ds queue_info;
    if (msgctl(msgid, IPC_STAT, &queue_info)) {
        perror("Ошибка получения статистики очереди");
        return;
    }

    printf("\n=== Информация об очереди ===\n");
    printf("Права доступа: %o\n", queue_info.msg_perm.mode);
    printf("Байт в очереди: %lu\n", queue_info.msg_cbytes);
    printf("Количество сообщений: %lu\n", queue_info.msg_qnum);
    printf("Макс. размер очереди: %lu байт\n", queue_info.msg_qbytes);
    
    printf("Последняя отправка: ");
    struct timeval stime = {queue_info.msg_stime, 0};
    print_time_with_ms(&stime);
    printf("\n");
    
    printf("Последнее получение: ");
    struct timeval rtime = {queue_info.msg_rtime, 0};
    print_time_with_ms(&rtime);
    printf("\n");
    
    printf("Последнее изменение: ");
    struct timeval ctime = {queue_info.msg_ctime, 0};
    print_time_with_ms(&ctime);
    printf("\n");
    
    printf("PID последнего отправителя: %d\n", queue_info.msg_lspid);
    printf("PID последнего получателя: %d\n", queue_info.msg_lrpid);
    printf("============================\n");
}

/* Функция отправки сообщения */
int send_message(int msgid, long msg_type, const char* text) {
    Message message = {
        .msg_type = msg_type,
        .sender_pid = getpid()
    };
    
    // Копируем текст сообщения с защитой от переполнения
    strncpy(message.msg_text, text, MAX_MSG_SIZE - 1);
    message.msg_text[MAX_MSG_SIZE - 1] = '\0';
    gettimeofday(&message.send_time, NULL);

    // Отправляем сообщение без ожидания (IPC_NOWAIT)
    if (msgsnd(msgid, &message, sizeof(message) - sizeof(long), IPC_NOWAIT) == -1) {
        perror("Ошибка отправки сообщения");
        return -1;
    }

    printf("Отправлено: [тип:%ld] %s (время: ", msg_type, message.msg_text);
    print_time_with_ms(&message.send_time);
    printf(", отправитель: %d)\n", message.sender_pid);
    return 0;
}

/* Функция получения сообщения */
int receive_message(int msgid, long msg_type, int flags) {
    Message message;
    struct timeval now;
    
    // Получаем сообщение с указанными параметрами
    ssize_t ret = msgrcv(msgid, &message, sizeof(message) - sizeof(long), 
                        msg_type, flags);
    
    if (ret == -1) {
        if (errno == ENOMSG) {
            printf("Нет сообщений типа %ld\n", msg_type);
        } else {
            perror("Ошибка при получении сообщения");
        }
        return -1;
    }
    
    // Рассчитываем задержку доставки
    gettimeofday(&now, NULL);
    long latency_ms = (now.tv_sec - message.send_time.tv_sec) * 1000 + 
                     (now.tv_usec - message.send_time.tv_usec) / 1000;
    
    printf("Получено: [тип:%ld] %s (отправлено: ", message.msg_type, message.msg_text);
    print_time_with_ms(&message.send_time);
    printf(", задержка: %ld мс, отправитель: %d)\n", latency_ms, message.sender_pid);
    return 0;
}

/* Эксперимент с приоритетами сообщений */
void run_priority_experiment(int msgid) {
    printf("\n*** Эксперимент с приоритетами ***\n");
    send_message(msgid, MSG_TYPE_PRIORITY_HIGH, "Сообщение с высоким приоритетом");
    send_message(msgid, MSG_TYPE_PRIORITY_MED, "Сообщение со средним приоритетом");
    send_message(msgid, MSG_TYPE_PRIORITY_LOW, "Сообщение с низким приоритетом");
    
    printf("\nПолучаем сообщение с наивысшим приоритетом:\n");
    receive_message(msgid, 0, 0);  // 0 означает получение сообщения с наименьшим типом
}

/* Эксперимент с fork() и межпроцессным взаимодействием */
void run_fork_experiment(int msgid) {
    printf("\n*** Эксперимент с fork() ***\n");
    pid_t pid = fork();
    
    if (pid == 0) {  // Дочерний процесс
        sleep(1);  // Имитация работы
        send_message(msgid, MSG_TYPE_CHILD, "Сообщение от дочернего процесса");
        exit(EXIT_SUCCESS);
    } else {  // Родительский процесс
        printf("Родительский процесс ожидает сообщение...\n");
        receive_message(msgid, MSG_TYPE_CHILD, 0);
        wait(NULL);  // Ожидаем завершения дочернего процесса
    }
}

/* Тест производительности очереди сообщений */
void run_performance_experiment(int msgid) {
    printf("\n*** Тест производительности ***\n");
    struct timeval start, end;
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < PERFORMANCE_TEST_MSGS; i++) {
        Message msg = {.msg_type = 1};
        if (msgsnd(msgid, &msg, 0, IPC_NOWAIT)) {
            perror("Ошибка в тесте производительности");
            break;
        }
    }
    gettimeofday(&end, NULL);
    
    // Расчет времени выполнения
    long elapsed = (end.tv_sec - start.tv_sec) * 1000000 + 
                  (end.tv_usec - start.tv_usec);
    printf("Отправлено %d сообщений за %ld мкс (%.3f мкс/сообщение)\n",
           PERFORMANCE_TEST_MSGS, elapsed, (float)elapsed / PERFORMANCE_TEST_MSGS);
    
    // Очистка очереди после теста
    while (msgrcv(msgid, NULL, 0, 0, IPC_NOWAIT) != -1) {}
}

/* Очистка очереди от всех сообщений */
void cleanup_queue(int msgid) {
    printf("\n*** Очистка очереди ***\n");
    int msg_count = 0;
    while (msgrcv(msgid, NULL, 0, 0, IPC_NOWAIT) != -1) {
        msg_count++;
    }
    printf("Удалено %d сообщений\n", msg_count);
}

int main() {
    key_t key;
    int msgid;

    printf("\n***** Демонстрация работы очередей сообщений *****\n");

    // Создаем ключ для очереди
    if ((key = ftok("/tmp", 'B')) == -1) {
        perror("Ошибка создания ключа");
        exit(EXIT_FAILURE);
    }

    // Создаем очередь сообщений
    if ((msgid = msgget(key, IPC_CREAT | QUEUE_PERMISSIONS)) == -1) {
        perror("Ошибка создания очереди");
        exit(EXIT_FAILURE);
    }

    printf("\nСоздана очередь с ID: %d\n", msgid);
    print_queue_info(msgid);

    /* Базовый эксперимент с разными типами сообщений */
    printf("\n*** Базовый эксперимент ***\n");
    for (int i = 1; i <= TOTAL_MESSAGES; i++) {
        char text[MAX_MSG_SIZE];
        snprintf(text, sizeof(text), "Тестовое сообщение %d", i);
        send_message(msgid, (i % 3) + 1, text);  // Типы 1, 2, 3
    }

    print_queue_info(msgid);

    /* Получение сообщений разных типов */
    printf("\nПолучаем сообщения типа 2:\n");
    for (int i = 0; i < 3; i++) {
        receive_message(msgid, MSG_TYPE_MED, IPC_NOWAIT);
    }

    printf("\nПопытка получить несуществующее сообщение типа 4:\n");
    receive_message(msgid, 4, IPC_NOWAIT);

    /* Дополнительные эксперименты */
    run_priority_experiment(msgid);
    run_fork_experiment(msgid);
    run_performance_experiment(msgid);

    /* Очистка и удаление очереди */
    cleanup_queue(msgid);

    if (msgctl(msgid, IPC_RMID, NULL)) {
        perror("Ошибка удаления очереди");
        exit(EXIT_FAILURE);
    }

    printf("\nОчередь %d успешно удалена\n", msgid);
    return EXIT_SUCCESS;
}