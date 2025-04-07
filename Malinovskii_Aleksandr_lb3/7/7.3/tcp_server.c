/******************************************************************************
 * 
 *  Цель:
 *      Продемонстрировать создание многопоточного TCP-сервера, который
 *      принимает подключения от нескольких клиентов одновременно, обрабатывая
 *      каждое соединение в отдельном потоке.
 *
 *  Описание:
 *      Сервер принимает число клиентов в качестве аргумента командной строки.
 *      После запуска, он ожидает подключения указанного числа клиентов.  Для
 *      каждого входящего соединения создается отдельный поток, который
 *      обрабатывает данные от клиента. Когда все клиенты подключились и
 *      обработали свои запросы, сервер завершает работу. Используется мьютекс
 *      для защиты доступа к общей переменной `clients_remaining`.
 *
 *  Основные компоненты:
 *      - `socket()`:  Создает сокет.
 *      - `bind()`: Привязывает сокет к определенному IP-адресу и порту.
 *      - `listen()`:  Начинает прослушивание входящих соединений.
 *      - `accept()`: Принимает входящее соединение.
 *      - `recv()`: Получает данные от клиента.
 *      - `close()`: Закрывает сокет.
 *      - `pthread_create()`: Создает новый поток.
 *      - `pthread_detach()`: Освобождает ресурсы потока после завершения.
 *      - `pthread_mutex_lock()`: Блокирует мьютекс.
 *      - `pthread_mutex_unlock()`: Разблокирует мьютекс.
 *      - `shutdown()`:  Прекращает передачу данных в сокете (чтение и
 *         запись).
 *      - `struct sockaddr_in`: Структура, содержащая информацию об адресе
 *        сервера и клиента (семейство протоколов, IP-адрес, порт).
 *
 *  Функции:
 *      - `handle_client(void *arg)`: Функция, выполняемая каждым потоком для
 *        обработки соединения с клиентом.
 *      - `main()`: Основная функция, создающая сокет, привязывающая его к
 *        адресу, начинающая прослушивание входящих соединений и создающая
 *        потоки для обработки каждого клиента.
 *
 *  Схема работы:
 *      1. Программа принимает число клиентов в качестве аргумента командной
 *         строки.
 *      2. Создается сокет и привязывается к указанному порту.
 *      3. Сервер начинает прослушивание входящих соединений.
 *      4. При каждом входящем соединении:
 *         - Принимается соединение с использованием `accept()`.
 *         - Создается новый поток, который выполняет функцию
 *           `handle_client()`.
 *         - Поток обрабатывает данные от клиента и закрывает соединение.
 *         - После завершения обработки клиентского соединения поток
 *           уменьшает счетчик оставшихся клиентов (`clients_remaining`) и,
 *           если это был последний клиент, завершает работу сервера.
 *      5. После того, как все клиенты подключились и были обработаны, сервер
 *         завершает работу.
 *
 *  Примечания:
 *      - Сервер принимает число клиентов, которых он должен обслужить, в
 *        качестве аргумента командной строки.
 *      - Используется мьютекс (`lock`) для защиты доступа к общей переменной
 *        `clients_remaining`.
 *      - Сервер использует порт 65432.
 *      - Сервер использует `shutdown()` для корректного завершения работы
 *        сокета, предотвращая дальнейшие подключения.
 *      - Не выполняется проверка на максимальное количество потоков.
 *      - Для компиляции необходимы заголовочные файлы `sys/socket.h`,
 *        `netinet/in.h`, `pthread.h` и `arpa/inet.h`.
 *      - Клиентский код должен подключаться к серверу на порту 65432.
 *
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 65432
#define BUFFER_SIZE 1024

// Объявляем server_fd как глобальную переменную
int server_fd;
int clients_remaining = 0;
pthread_mutex_t lock;

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
      //  printf("[TCP] Client connected: %s\n", buffer);
    }
    
    close(client_socket);
    
    pthread_mutex_lock(&lock);
    clients_remaining--;
    if (clients_remaining <= 0) {
        printf("[TCP] All clients connected, shutting down\n");
        shutdown(server_fd, SHUT_RDWR);  // Теперь server_fd доступна
    }
    pthread_mutex_unlock(&lock);
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    clients_remaining = atoi(argv[1]);
    pthread_mutex_init(&lock, NULL);
    
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1000) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[TCP] Server listening on port %d for %d clients\n", PORT, clients_remaining);

    while (clients_remaining > 0) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (clients_remaining > 0) {
                perror("accept");
            }
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, &client_socket) < 0) {
            perror("could not create thread");
            close(client_socket);
        }
        pthread_detach(thread_id);
    }

    close(server_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}