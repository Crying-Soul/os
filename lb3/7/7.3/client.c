#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>

#define TCP_PORT 65432
#define UDP_PORT 65433
#define BUFFER_SIZE 1024
#define MAX_CONNECTION_ATTEMPTS 3
#define CONNECTION_TIMEOUT_SEC 5

void print_error(const char *context) {
    fprintf(stderr, "[ERROR] %s: %s\n", context, strerror(errno));
}

bool initialize_tcp_socket(int *sock) {
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock < 0) {
        print_error("TCP socket creation");
        return false;
    }
    
    // Установка таймаута на подключение
    struct timeval timeout = {.tv_sec = CONNECTION_TIMEOUT_SEC, .tv_usec = 0};
    if (setsockopt(*sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        print_error("Setting TCP socket timeout");
        close(*sock);
        return false;
    }
    
    return true;
}

bool connect_to_tcp_server(int sock, const char *server_ip) {
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TCP_PORT)
    };
    
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
        return false;
    }
    
    for (int attempt = 1; attempt <= MAX_CONNECTION_ATTEMPTS; attempt++) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
            return true;
        }
        
        if (attempt < MAX_CONNECTION_ATTEMPTS) {
            fprintf(stderr, "Connection attempt %d failed, retrying...\n", attempt);
            sleep(1);
        }
    }
    
    print_error("TCP connection");
    return false;
}

void tcp_client(const char* client_id, const char* server_ip) {
    int sock = -1;
    
    if (!initialize_tcp_socket(&sock)) {
        return;
    }
    
    if (!connect_to_tcp_server(sock, server_ip)) {
        close(sock);
        return;
    }
    
    char message[BUFFER_SIZE];
    int msg_len = snprintf(message, BUFFER_SIZE, "Client %s connected via TCP", client_id);
    
    ssize_t bytes_sent = send(sock, message, msg_len, 0);
    if (bytes_sent < 0) {
        print_error("TCP send");
    } else {
        printf("[TCP] Sent %zd/%d bytes: %s\n", bytes_sent, msg_len, message);
        
        // Попытка получить ответ от сервера
        char response[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(sock, response, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("[TCP] Received: %s\n", response);
        } else if (bytes_received == 0) {
            printf("[TCP] Server closed connection\n");
        } else {
            print_error("TCP receive");
        }
    }
    
    close(sock);
}

bool initialize_udp_socket(int *sock) {
    *sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sock < 0) {
        print_error("UDP socket creation");
        return false;
    }
    return true;
}

void udp_client(const char* client_id, const char* server_ip) {
    int sock = -1;
    
    if (!initialize_udp_socket(&sock)) {
        return;
    }
    
    struct sockaddr_in servaddr = {
        .sin_family = AF_INET,
        .sin_port = htons(UDP_PORT)
    };
    
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
        close(sock);
        return;
    }
    
    char message[BUFFER_SIZE];
    int msg_len = snprintf(message, BUFFER_SIZE, "Client %s connected via UDP", client_id);
    
    ssize_t bytes_sent = sendto(sock, message, msg_len, 0,
                               (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if (bytes_sent < 0) {
        print_error("UDP send");
    } else {
        printf("[UDP] Sent %zd/%d bytes: %s\n", bytes_sent, msg_len, message);
        
        // Попытка получить ответ от сервера (с таймаутом)
        struct timeval timeout = {.tv_sec = CONNECTION_TIMEOUT_SEC, .tv_usec = 0};
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            print_error("Setting UDP receive timeout");
        } else {
            char response[BUFFER_SIZE] = {0};
            socklen_t addr_len = sizeof(servaddr);
            ssize_t bytes_received = recvfrom(sock, response, BUFFER_SIZE - 1, 0,
                                            (struct sockaddr *)&servaddr, &addr_len);
            if (bytes_received > 0) {
                response[bytes_received] = '\0';
                printf("[UDP] Received: %s\n", response);
            } else if (bytes_received == 0) {
                printf("[UDP] Empty response from server\n");
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[UDP] No response from server (timeout)\n");
            } else {
                print_error("UDP receive");
            }
        }
    }
    
    close(sock);
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s --tcp|--udp [--server <ip>] <client_id>\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --tcp        Use TCP protocol\n");
    fprintf(stderr, "  --udp        Use UDP protocol\n");
    fprintf(stderr, "  --server     Server IP address (default: 127.0.0.1)\n");
    fprintf(stderr, "  <client_id>  Client identifier string\n");
}

int main(int argc, char *argv[]) {
    bool tcp_flag = false;
    bool udp_flag = false;
    const char* client_id = NULL;
    const char* server_ip = "127.0.0.1"; // Значение по умолчанию
    
    static struct option long_options[] = {
        {"tcp", no_argument, NULL, 't'},
        {"udp", no_argument, NULL, 'u'},
        {"server", required_argument, NULL, 's'},
        {NULL, 0, NULL, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "tus:", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                tcp_flag = true;
                break;
            case 'u':
                udp_flag = true;
                break;
            case 's':
                server_ip = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    
    if (optind < argc) {
        client_id = argv[optind];
    } else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    if (tcp_flag && udp_flag) {
        fprintf(stderr, "Error: Cannot specify both --tcp and --udp\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    if (!tcp_flag && !udp_flag) {
        fprintf(stderr, "Error: Must specify either --tcp or --udp\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    if (tcp_flag) {
        tcp_client(client_id, server_ip);
    } else {
        udp_client(client_id, server_ip);
    }
    
    return EXIT_SUCCESS;
}