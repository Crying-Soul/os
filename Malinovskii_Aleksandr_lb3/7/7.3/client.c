#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void tcp_client(const char* client_id) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(65432);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported\n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nTCP Connection Failed\n");
        return;
    }

    //snprintf(message, BUFFER_SIZE, "Client %s connected via TCP", client_id);
    send(sock, message, strlen(message), 0);
    close(sock);
}

void udp_client(const char* client_id) {
    int sockfd;
    struct sockaddr_in servaddr;
    char message[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(65433);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    snprintf(message, BUFFER_SIZE, "Client %s connected via UDP", client_id);
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    close(sockfd);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <tcp|udp> <client_id>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "tcp") == 0) {
        tcp_client(argv[2]);
    } else if (strcmp(argv[1], "udp") == 0) {
        udp_client(argv[2]);
    } else {
        printf("Invalid protocol. Use 'tcp' or 'udp'\n");
        return 1;
    }

    return 0;
}