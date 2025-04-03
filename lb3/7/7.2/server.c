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
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // Создание UDP сокета
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Привязка сокета к адресу
    if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    printf("UDP сервер слушает на порту %d...\n", PORT);fflush(stdout);
    
    while (1) {
        // Получение данных от клиента
        int n = recvfrom(server_fd, (char *)buffer, BUFFER_SIZE, 
                         MSG_WAITALL, (struct sockaddr *)&client_addr,
                         &client_len);
        buffer[n] = '\0';
        
        printf("Получено от %s:%d: %s\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port), buffer);fflush(stdout);
        
        // Отправка ответа клиенту
        char *response = "Сообщение получено сервером (UDP)";
        sendto(server_fd, response, strlen(response), 
               MSG_CONFIRM, (const struct sockaddr *)&client_addr,
               client_len);
    }
    
    close(server_fd);
    return 0;
}