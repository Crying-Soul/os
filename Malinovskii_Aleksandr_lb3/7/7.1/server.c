/******************************************************************************
 * 
 *  Цель:
 *      Продемонстрировать создание простого TCP сервера, который принимает
 *      подключения от клиентов, получает сообщения и отправляет ответы.
 *
 *  Описание:
 *      Программа создает сокет, привязывает его к указанному порту, начинает
 *      прослушивание входящих соединений. При подключении клиента, сервер
 *      принимает данные, выводит их на экран, отправляет ответ и закрывает
 *      соединение.
 *
 *  Основные компоненты:
 *      - `socket()`:  Создает сокет.
 *      - `bind()`: Привязывает сокет к определенному IP-адресу и порту.
 *      - `listen()`:  Начинает прослушивание входящих соединений.
 *      - `accept()`: Принимает входящее соединение.
 *      - `send()`: Отправляет данные клиенту.
 *      - `read()`: Читает данные от клиента.
 *      - `close()`: Закрывает сокет.
 *      - `struct sockaddr_in`: Структура, содержащая информацию об адресе
 *        сервера (семейство протоколов, IP-адрес, порт).
 *
 *  Схема работы:
 *      1. Создается сокет с использованием `socket()`.
 *      2. Настраиваются опции сокета с использованием `setsockopt()`.
 *      3. Заполняется структура `sockaddr_in` информацией об адресе сервера.
 *      4. Сокет привязывается к адресу с использованием `bind()`.
 *      5. Сервер начинает прослушивание входящих соединений с использованием
 *         `listen()`.
 *      6. Принимается входящее соединение с использованием `accept()`.
 *      7. Получаются данные от клиента с использованием `read()`.
 *      8. Отправляется ответ клиенту с использованием `send()`.
 *      9. Закрываются сокеты (новый и слушающий) с использованием `close()`.
 *
 *  Примечания:
 *      - Сервер принимает только одно соединение.
 *      - Порт сервера задан константой PORT (12345).
 *      - Адрес сервера (INADDR_ANY) означает, что сервер будет принимать
 *        соединения на всех доступных IP-адресах.
 *      - Используется `setsockopt` с `SO_REUSEADDR` и `SO_REUSEPORT`, чтобы
 *        позволить повторное использование адреса и порта.
 *      - Размер буфера для приема данных ограничен BUFFER_SIZE (1024 байта).
 *      - В программе не предусмотрена обработка ошибок при разрыве соединения
 *        или других сетевых проблемах.
 *      - Для компиляции необходимы заголовочные файлы `sys/socket.h`,
 *        `netinet/in.h` и `asm-generic/socket.h`.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Настройка опций сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Привязка сокета к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Ожидание подключений
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер слушает на порту %d...\n", PORT);
    
    // Принятие входящего соединения
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    // Чтение данных от клиента
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Получено от клиента: %s\n", buffer);
    
    // Отправка ответа клиенту
    char *response = "Сообщение получено сервером";
    send(new_socket, response, strlen(response), 0);
    printf("Ответ отправлен клиенту\n");
    
    close(new_socket);
    close(server_fd);
    return 0;
}