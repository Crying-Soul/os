#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;
    
    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Socket creation error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Преобразование IP-адреса
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address/ Address not supported: %s\n", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }
    
    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Connection Failed: %s\n", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }
    
    // Отправка сообщения серверу
    const char *message = "Привет, сервер!";
    ssize_t bytes_sent = send(sock, message, strlen(message), 0);
    if (bytes_sent < 0) {
        fprintf(stderr, "Send failed: %s\n", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }
    printf("Сообщение отправлено серверу (%zd байт)\n", bytes_sent);
    
    // Чтение ответа от сервера
    bytes_read = read(sock, buffer, BUFFER_SIZE - 1); // Оставляем место для нуль-терминатора
    if (bytes_read < 0) {
        fprintf(stderr, "Read failed: %s\n", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0'; // Гарантируем нуль-терминацию строки
    printf("Ответ сервера: %s\n", buffer);
    
    close(sock);
    return EXIT_SUCCESS;
}