#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    int fd = open("cmd_pipe", O_WRONLY);
    char msg[] = "Тестовое сообщение через FIFO";
    write(fd, msg, sizeof(msg));
    close(fd);
    return 0;
}