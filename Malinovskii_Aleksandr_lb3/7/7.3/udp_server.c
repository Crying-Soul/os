/******************************************************************************
 *
 *  Цель:
 *      Продемонстрировать создание простого многопоточного UDP-сервера,
 *      который принимает сообщения от указанного числа клиентов и затем
 *      завершает работу.  В данном примере многопоточность используется не
 *      для параллельной обработки каждого клиента, а для ожидания подключений
 *      от нескольких клиентов одновременно.
 *
 *  Описание:
 *      Сервер принимает число клиентов в качестве аргумента командной строки.
 *      Он создает UDP-сокет и ждет получения сообщения от каждого клиента.
 *      После получения сообщения от заданного числа клиентов, сервер
 *      завершает работу.  Многопоточность используется, чтобы продемонстрировать
 *      одновременное ожидание сообщений от нескольких клиентов, хотя в данном
 *      примере каждый клиент обрабатывается последовательно, а не
 *      параллельно. Мьютекс используется для защиты доступа к общей переменной
 *      `clients_remaining`.
 *
 *  Основные компоненты:
 *      - `socket()`:  Создает UDP-сокет.
 *      - `bind()`: Привязывает сокет к определенному IP-адресу и порту.
 *      - `recvfrom()`: Получает данные от клиента.
 *      - `close()`: Закрывает сокет.
 *      - `pthread_mutex_lock()`: Блокирует мьютекс.
 *      - `pthread_mutex_unlock()`: Разблокирует мьютекс.
 *      - `struct sockaddr_in`: Структура, содержащая информацию об адресе
 *        сервера и клиента (семейство протоколов, IP-адрес, порт).
 *
 *  Схема работы:
 *      1. Программа принимает число клиентов в качестве аргумента командной
 *         строки.
 *      2. Создается UDP-сокет и привязывается к указанному порту.
 *      3. Сервер ожидает сообщения от каждого клиента, используя
 *         `recvfrom()`.
 *      4. После получения сообщения от каждого клиента, значение переменной
 *         `clients_remaining` уменьшается.
 *      5. Когда `clients_remaining` становится равным 0, сервер завершает
 *         работу.
 *
 *  Примечания:
 *      - Сервер принимает число клиентов, которых он должен обслужить, в
 *        качестве аргумента командной строки.
 *      - Используется мьютекс (`lock`) для защиты доступа к общей переменной
 *        `clients_remaining`.
 *      - Сервер использует порт 65433.
 *      - В данном примере не создаются отдельные потоки для обработки каждого
 *        клиента. UDP-дейтаграммы обрабатываются последовательно.
 *      - Сокет привязывается к `INADDR_ANY`, что позволяет принимать
 *        соединения с любого IP-адреса.
 *      - Не выполняется проверка на максимальное количество клиентов.
 *      - Нет обработки ошибок, связанных с обрывом соединения.
 *      - Для компиляции необходимы заголовочные файлы `sys/socket.h`,
 *        `netinet/in.h` и `arpa/inet.h`.
 *      - Клиентский код должен отправлять дейтаграммы на порт 65433.
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

#define PORT 65433
#define BUFFER_SIZE 1024

int clients_remaining = 0;
pthread_mutex_t lock;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    clients_remaining = atoi(argv[1]);
    pthread_mutex_init(&lock, NULL);
    
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("[UDP] Server listening on port %d for %d clients\n", PORT, clients_remaining);

    while (clients_remaining > 0) {
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            
            pthread_mutex_lock(&lock);
            clients_remaining--;
            pthread_mutex_unlock(&lock);
        }
    }
    
    printf("[UDP] All clients connected, shutting down\n");
    close(sockfd);
    pthread_mutex_destroy(&lock);
    
    return 0;
}