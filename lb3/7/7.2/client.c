#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_RETRIES 3
#define TIMEOUT_SEC 5

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(serv_addr);
    
    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Установка таймаута на получение данных
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Отправка сообщения серверу
    const char *message = "Привет, сервер! (UDP)";
    size_t message_len = strlen(message);
    
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        ssize_t sent_bytes = sendto(sockfd, message, message_len, 
                                   MSG_CONFIRM, (const struct sockaddr *)&serv_addr,
                                   sizeof(serv_addr));
        
        if (sent_bytes < 0) {
            perror("sendto failed");
            continue;
        } else if ((size_t)sent_bytes != message_len) {
            fprintf(stderr, "Sent partial message\n");
            continue;
        }
        
        printf("Сообщение отправлено серверу");fflush(stdout);
        
        // Получение ответа от сервера
        ssize_t received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 
                                        0, (struct sockaddr *)&serv_addr, &addr_len);
        
        if (received_bytes < 0) {
            perror("recvfrom failed or timeout");
            continue;
        }
        
        buffer[received_bytes] = '\0';
        printf("Ответ сервера: %s\n", buffer);fflush(stdout);
        break;
    }
    
    close(sockfd);
    return 0;
}