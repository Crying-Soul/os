#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    const char *pipe_name = "cmd_pipe";
    const char *msg = "Тестовое сообщение через FIFO";
    
    // Открываем именованный канал для записи
    int fd = open(pipe_name, O_WRONLY);
    if (fd == -1) {
        perror("Ошибка при открытии именованного канала");
        exit(EXIT_FAILURE);
    }
    
    // Записываем сообщение (без нулевого терминатора)
    ssize_t bytes_written = write(fd, msg, strlen(msg));
    if (bytes_written == -1) {
        perror("Ошибка при записи в канал");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    // Закрываем канал
    if (close(fd) == -1) {
        perror("Ошибка при закрытии канала");
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}