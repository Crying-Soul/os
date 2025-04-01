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