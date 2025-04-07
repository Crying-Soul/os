/******************************************************************************
 * 
 *  Цель:
 *      Продемонстрировать основные и расширенные возможности очередей
 *      сообщений System V (message queues) в Linux, включая отправку,
 *      получение сообщений разных типов, получение информации об очереди, а
 *      также эксперименты с приоритетами, fork() и производительностью.
 *
 *  Описание:
 *      Программа выполняет следующие действия:
 *          1. Создает очередь сообщений System V.
 *          2. Отправляет несколько сообщений разных типов в очередь.
 *          3. Получает сообщения определенного типа из очереди.
 *          4. Выводит информацию об очереди (права доступа, количество
 *             сообщений, размер и т.д.).
 *          5. Проводит эксперимент с приоритетами, отправляя сообщения с
 *             разными типами и получая их в порядке приоритета.
 *          6. Проводит эксперимент с fork(), отправляя сообщение из дочернего
 *             процесса и получая его в родительском.
 *          7. Проводит эксперимент с производительностью, измеряя время
 *             отправки большого количества сообщений.
 *          8. Очищает и удаляет очередь сообщений.
 *
 *  Основные компоненты:
 *      - `msgget()`:  Создает или получает идентификатор существующей очереди
 *         сообщений.
 *      - `msgsnd()`: Отправляет сообщение в очередь.
 *      - `msgrcv()`: Получает сообщение из очереди.
 *      - `msgctl()`: Выполняет различные операции управления над очередью,
 *        такие как получение информации и удаление очереди.
 *      - `struct msqid_ds`: Структура, содержащая информацию об очереди.
 *      - `struct msg_buffer`: Структура, определяющая формат сообщения,
 *        содержащего тип сообщения, текст и временную метку.
 *
 *  Функции:
 *      - `print_queue_info()`: Выводит информацию об очереди сообщений.
 *      - `send_message()`: Отправляет сообщение в очередь.
 *      - `receive_message()`: Получает сообщение из очереди.
 *      - `experiment_priority()`:  Демонстрирует работу с приоритетами
 *         сообщений.
 *      - `experiment_fork()`: Демонстрирует использование очередей сообщений
 *         между процессами, созданными с помощью fork().
 *      - `experiment_performance()`:  Измеряет производительность отправки
 *         сообщений.
 *      - `print_time_with_ms()`: Форматирует вывод времени с миллисекундами.
 *
 *  Эксперименты:
 *      - **Приоритеты**: Демонстрируется возможность получения сообщений в
 *        порядке приоритета (на основе типа сообщения).
 *      - **Взаимодействие процессов**:  Показывается, как родительский и
 *        дочерний процессы могут обмениваться сообщениями через очередь.
 *      - **Производительность**:  Измеряется время, необходимое для отправки
 *         большого количества сообщений, позволяя оценить производительность
 *         механизма очередей.
 *
 *  Примечания:
 *      - Для компиляции программы необходима библиотека `sys/msg.h`.
 *      - Ключ для создания очереди сообщений генерируется с использованием
 *        `ftok()`.
 *      - Программа очищает и удаляет очередь сообщений после завершения работы.
 *
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>  // Для gettimeofday()

#define MAX_MSG_SIZE 256
#define MSG_TYPE_1 1
#define MSG_TYPE_2 2
#define MSG_TYPE_3 3
#define TOTAL_MESSAGES 8

struct msg_buffer {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
    struct timeval send_time;  // Добавляем временную метку
};

// Функция для преобразования времени с миллисекундами
void print_time_with_ms(struct timeval tv) {
    if (tv.tv_sec == 0 && tv.tv_usec == 0) {
        printf("никогда");
        return;
    }
    
    struct tm *timeinfo;
    char buffer[80];
    
    timeinfo = localtime(&tv.tv_sec);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s.%03ld", buffer, tv.tv_usec / 1000);
}

void print_queue_info(int msgid) {
    struct msqid_ds queue_info;
    if (msgctl(msgid, IPC_STAT, &queue_info) == -1) {
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
    print_time_with_ms(stime);
    printf("\n");
    
    printf("Последнее получение: ");
    struct timeval rtime = {queue_info.msg_rtime, 0};
    print_time_with_ms(rtime);
    printf("\n");
    
    printf("Последнее изменение: ");
    struct timeval ctime = {queue_info.msg_ctime, 0};
    print_time_with_ms(ctime);
    printf("\n");
    
    printf("PID последнего отправителя: %d\n", queue_info.msg_lspid);
    printf("PID последнего получателя: %d\n", queue_info.msg_lrpid);
    printf("============================\n");
}

void send_message(int msgid, long msg_type, const char* text) {
    struct msg_buffer message;
    message.msg_type = msg_type;
    strncpy(message.msg_text, text, MAX_MSG_SIZE - 1);
    gettimeofday(&message.send_time, NULL);  // Записываем текущее время
    
    if (msgsnd(msgid, &message, sizeof(message.msg_text) + sizeof(message.send_time), 0) == -1) {
        perror("Ошибка отправки сообщения");
    } else {
        printf("Отправлено: [тип:%ld] %s (время: ", msg_type, message.msg_text);
        print_time_with_ms(message.send_time);
        printf(")\n");
    }
}

void receive_message(int msgid, long msg_type, int flags) {
    struct msg_buffer message;
    struct timeval now;
    
    ssize_t ret = msgrcv(msgid, &message, sizeof(message.msg_text) + sizeof(message.send_time), 
             msg_type, flags);
    
    if (ret == -1) {
        if (errno == ENOMSG) {
            printf("Нет сообщений типа %ld\n", msg_type);
        } else {
            perror("Ошибка при получении сообщения");
        }
        return;
    }
    
    gettimeofday(&now, NULL);
    long latency_ms = (now.tv_sec - message.send_time.tv_sec) * 1000 + 
                     (now.tv_usec - message.send_time.tv_usec) / 1000;
    
    printf("Получено: [тип:%ld] %s (отправлено: ", message.msg_type, message.msg_text);
    print_time_with_ms(message.send_time);
    printf(", задержка: %ld мс)\n", latency_ms);
}

void experiment_priority(int msgid) {
    printf("\n*** Эксперимент с приоритетами ***\n");
    send_message(msgid, 10, "Сообщение с высоким приоритетом (тип 10)");
    send_message(msgid, 5, "Сообщение со средним приоритетом (тип 5)");
    send_message(msgid, 1, "Сообщение с низким приоритетом (тип 1)");
    
    printf("\nПолучаем сообщение с наивысшим приоритетом:\n");
    receive_message(msgid, 0, 0);  // 0 означает "получить сообщение с наименьшим типом"
}

void experiment_fork(int msgid) {
    printf("\n*** Эксперимент с fork() ***\n");
    pid_t pid = fork();
    
    if (pid == 0) {  // Дочерний процесс
        sleep(1);
        send_message(msgid, MSG_TYPE_3, "Сообщение от дочернего процесса");
        exit(0);
    } else {  // Родительский процесс
        printf("Родительский процесс ждет сообщение...\n");
        receive_message(msgid, MSG_TYPE_3, 0);
        wait(NULL);
    }
}

void experiment_performance(int msgid) {
    printf("\n*** Эксперимент с производительностью ***\n");
    struct timeval start, end;
    const int num_msgs = 100;
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < num_msgs; i++) {
        struct msg_buffer msg;
        msg.msg_type = 1;
        msgsnd(msgid, &msg, 0, IPC_NOWAIT);
    }
    gettimeofday(&end, NULL);
    
    long elapsed = (end.tv_sec - start.tv_sec) * 1000000 + 
                  (end.tv_usec - start.tv_usec);
    printf("Отправка %d сообщений заняла %ld мкс (%.3f мкс/сообщение)\n",
           num_msgs, elapsed, (float)elapsed / num_msgs);
    
    // Очистка
    while (msgrcv(msgid, NULL, 0, 0, IPC_NOWAIT) != -1) {}
}

int main() {
    key_t key;
    int msgid;

    printf("\n***** Расширенная демонстрация очередей сообщений *****\n");

    // Создание уникального ключа
    if ((key = ftok("/tmp", 'B')) == -1) {
        perror("Ошибка создания ключа");
        exit(EXIT_FAILURE);
    }

    // Создание очереди сообщений
    if ((msgid = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("Ошибка создания очереди");
        exit(EXIT_FAILURE);
    }

    printf("\nСоздана очередь с идентификатором: %d\n", msgid);
    print_queue_info(msgid);

    // Базовый эксперимент с разными типами сообщений
    printf("\n*** Базовый эксперимент ***\n");
    for (int i = 1; i <= TOTAL_MESSAGES; i++) {
        char text[MAX_MSG_SIZE];
        snprintf(text, sizeof(text), "Тестовое сообщение %d", i);
        send_message(msgid, (i % 3) + 1, text);  // Типы 1, 2, 3
    }

    print_queue_info(msgid);

    // Получение сообщений разных типов
    printf("\nПолучаем сообщения типа 2:\n");
    for (int i = 0; i < 3; i++) {
        receive_message(msgid, 2, IPC_NOWAIT);
    }

    printf("\nПопытка получить сообщение типа 4 (не существует):\n");
    receive_message(msgid, 4, IPC_NOWAIT);

    // Дополнительные эксперименты
    experiment_priority(msgid);
    experiment_fork(msgid);
    experiment_performance(msgid);

    // Очистка очереди
    printf("\n*** Очистка очереди ***\n");
    int msg_count = 0;
    while (msgrcv(msgid, NULL, 0, 0, IPC_NOWAIT) != -1) {
        msg_count++;
    }
    printf("Удалено %d сообщений\n", msg_count);

    // Удаление очереди
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Ошибка удаления очереди");
    } else {
        printf("\nОчередь %d успешно удалена\n", msgid);
    }

    return EXIT_SUCCESS;
}