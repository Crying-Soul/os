#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    
    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Отправка сообщения серверу
    char *message = "Привет, сервер! (UDP)";
    sendto(sockfd, message, strlen(message), 
           MSG_CONFIRM, (const struct sockaddr *)&serv_addr,
           sizeof(serv_addr));
    printf("Сообщение отправлено серверу\n");
    
    // Получение ответа от сервера
    int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 
                     MSG_WAITALL, NULL, NULL);
    buffer[n] = '\0';
    printf("Ответ сервера: %s\n", buffer);
    
    close(sockfd);
    return 0;
}