#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <arpa/inet.h>

#define TCP_PORT 65432
#define UDP_PORT 65433
#define BUFFER_SIZE 1024
#define MAX_PENDING_CONNECTIONS 1000
#define THREAD_STACK_SIZE (256 * 1024) // 256KB stack per thread
#define TIMEOUT_SEC 1

typedef struct {
    int server_fd;
    volatile int clients_remaining;
    pthread_mutex_t lock;
    bool shutdown_requested;
} server_state_t;

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    server_state_t *state;
} tcp_client_t;

static volatile sig_atomic_t global_shutdown = 0;

void handle_signal(int sig) {
    (void)sig; // Подавляем предупреждение о неиспользуемом параметре
    global_shutdown = 1;
}

void cleanup_tcp_client(void *arg) {
    tcp_client_t *client = (tcp_client_t *)arg;
    if (client) {
        close(client->client_socket);
        free(client);
    }
}

void *handle_tcp_client(void *arg) {
    tcp_client_t *client = (tcp_client_t *)arg;
    server_state_t *state = client->state;

    pthread_cleanup_push(cleanup_tcp_client, client);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client->client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("[TCP] Received from %s: %s\n",
               inet_ntoa(client->client_addr.sin_addr),
               buffer);

        const char *response = "Message received";
        if (send(client->client_socket, response, strlen(response), 0) < 0) {
            perror("[TCP] Send failed");
        }
    } else if (bytes_received == 0) {
        printf("[TCP] Client disconnected: %s\n", inet_ntoa(client->client_addr.sin_addr));
    } else {
        perror("[TCP] Receive error");
    }

    pthread_mutex_lock(&state->lock);
    state->clients_remaining--;
    bool should_shutdown = (state->clients_remaining <= 0) && !state->shutdown_requested;
    if (should_shutdown) {
        state->shutdown_requested = true;
    }
    pthread_mutex_unlock(&state->lock);

    if (should_shutdown) {
        printf("[TCP] All clients served, initiating shutdown\n");
        shutdown(state->server_fd, SHUT_RDWR);
    }

    pthread_cleanup_pop(1); // Выполнит cleanup_tcp_client и освободит память
    return NULL;
}

void setup_tcp_socket(int *server_fd, struct sockaddr_in *address) {
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[TCP] Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    // Устанавливаем SO_REUSEADDR
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[TCP] setsockopt SO_REUSEADDR failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
#ifdef SO_REUSEPORT
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("[TCP] setsockopt SO_REUSEPORT failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
#endif

    if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("[TCP] Bind failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("[TCP] Listen failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
}

void start_tcp_server(int num_clients) {
    server_state_t state = {
        .clients_remaining = num_clients,
        .server_fd = -1,
        .shutdown_requested = false
    };

    if (pthread_mutex_init(&state.lock, NULL) != 0) {
        perror("[TCP] Mutex init failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(TCP_PORT)
    };

    setup_tcp_socket(&state.server_fd, &address);

    printf("[TCP] Server listening on port %d for %d clients\n", TCP_PORT, num_clients);fflush(stdout);

    while (!global_shutdown) {
        pthread_mutex_lock(&state.lock);
        bool continue_accepting = (state.clients_remaining > 0) && !state.shutdown_requested;
        pthread_mutex_unlock(&state.lock);

        if (!continue_accepting)
            break;

        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(state.server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_socket < 0) {
            if (errno == EINVAL || errno == EBADF) {
                break; // Серверный сокет закрыт
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("[TCP] Accept failed");
            }
            continue;
        }

        printf("[TCP] New connection from %s\n", inet_ntoa(client_addr.sin_addr));fflush(stdout);

        tcp_client_t *client = malloc(sizeof(tcp_client_t));
        if (!client) {
            perror("[TCP] malloc failed");
            close(client_socket);
            continue;
        }
        client->client_socket = client_socket;
        client->client_addr = client_addr;
        client->state = &state;

        pthread_attr_t thread_attr;
        if (pthread_attr_init(&thread_attr) != 0) {
            perror("[TCP] pthread_attr_init failed");
            free(client);
            close(client_socket);
            continue;
        }
        if (pthread_attr_setstacksize(&thread_attr, THREAD_STACK_SIZE) != 0) {
            perror("[TCP] pthread_attr_setstacksize failed");
            pthread_attr_destroy(&thread_attr);
            free(client);
            close(client_socket);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, &thread_attr, handle_tcp_client, client) != 0) {
            perror("[TCP] Thread creation failed");
            pthread_attr_destroy(&thread_attr);
            free(client);
            close(client_socket);
            continue;
        }

        pthread_attr_destroy(&thread_attr);
        pthread_detach(thread_id);
    }

    // Ожидаем завершения всех клиентских потоков (можно заменить условной переменной)
    while (true) {
        pthread_mutex_lock(&state.lock);
        int remaining = state.clients_remaining;
        pthread_mutex_unlock(&state.lock);
        if (remaining <= 0)
            break;
        usleep(100000); // 100 мс
    }

    if (state.server_fd >= 0)
        close(state.server_fd);
    pthread_mutex_destroy(&state.lock);
    printf("[TCP] Server shutdown complete\n");
}

void start_udp_server(int num_clients) {
    server_state_t state = {
        .clients_remaining = num_clients,
        .server_fd = -1,
        .shutdown_requested = false
    };

    if (pthread_mutex_init(&state.lock, NULL) != 0) {
        perror("[UDP] Mutex init failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(UDP_PORT)
    };

    if ((state.server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[UDP] Socket creation failed");
        pthread_mutex_destroy(&state.lock);
        exit(EXIT_FAILURE);
    }

    struct timeval timeout = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };
    if (setsockopt(state.server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("[UDP] setsockopt failed");
        close(state.server_fd);
        pthread_mutex_destroy(&state.lock);
        exit(EXIT_FAILURE);
    }

    if (bind(state.server_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("[UDP] Bind failed");
        close(state.server_fd);
        pthread_mutex_destroy(&state.lock);
        exit(EXIT_FAILURE);
    }

    printf("[UDP] Server listening on port %d for %d clients\n", UDP_PORT, num_clients);fflush(stdout);

    while (!global_shutdown) {
        pthread_mutex_lock(&state.lock);
        bool continue_receiving = (state.clients_remaining > 0) && !state.shutdown_requested;
        pthread_mutex_unlock(&state.lock);
        if (!continue_receiving)
            break;

        char buffer[BUFFER_SIZE] = {0};
        struct sockaddr_in cliaddr = {0};
        socklen_t len = sizeof(cliaddr);
        ssize_t n = recvfrom(state.server_fd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&cliaddr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[UDP] Received from %s: %s\n", inet_ntoa(cliaddr.sin_addr), buffer);fflush(stdout);

            const char *response = "UDP message received";
            if (sendto(state.server_fd, response, strlen(response), 0,
                       (const struct sockaddr *)&cliaddr, len) < 0) {
                perror("[UDP] sendto failed");
            }

            pthread_mutex_lock(&state.lock);
            state.clients_remaining--;
            if (state.clients_remaining <= 0)
                state.shutdown_requested = true;
            pthread_mutex_unlock(&state.lock);
        } else if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("[UDP] recvfrom error");
            }
        }
    }

    if (state.server_fd >= 0)
        close(state.server_fd);
    pthread_mutex_destroy(&state.lock);
    printf("[UDP] Server shutdown complete\n");
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s --tcp|--udp <num_clients>\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --tcp        Start TCP server\n");
    fprintf(stderr, "  --udp        Start UDP server\n");
    fprintf(stderr, "  <num_clients> Number of clients to wait for (positive integer)\n");
}

int main(int argc, char *argv[]) {
    struct sigaction sa = {
        .sa_handler = handle_signal,
        .sa_flags = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    bool tcp_flag = false;
    bool udp_flag = false;
    int num_clients = 0;

    static struct option long_options[] = {
        {"tcp", no_argument, NULL, 't'},
        {"udp", no_argument, NULL, 'u'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "tuh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                tcp_flag = true;
                break;
            case 'u':
                udp_flag = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind < argc) {
        char *endptr;
        long clients = strtol(argv[optind], &endptr, 10);
        if (*endptr != '\0' || clients <= 0) {
            fprintf(stderr, "Error: Number of clients must be a positive integer\n");
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        num_clients = (int)clients;
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
        start_tcp_server(num_clients);
    } else {
        start_udp_server(num_clients);
    }

    return EXIT_SUCCESS;
}
