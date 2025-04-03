#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define PIPE_NAME "cmd_pipe"

int main() {
    int fd = open(PIPE_NAME, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка при открытии именованного канала");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = {0};  // Инициализация нулями
    
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // Оставляем место для нуль-терминатора
    if (bytes_read == -1) {
        perror("Ошибка при чтении из канала");
        close(fd);
        exit(EXIT_FAILURE);
    } else if (bytes_read == 0) {
        fprintf(stderr, "Достигнут конец файла (канал закрыт писателем)\n");
    } else {
        buffer[bytes_read] = '\0'; // Гарантируем нуль-терминацию
        printf("Получено: %s\n", buffer);
    }

    if (close(fd) == -1) {
        perror("Ошибка при закрытии канала");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}