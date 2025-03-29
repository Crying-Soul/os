#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd = open("cmd_pipe", O_RDONLY);
    char buffer[100];
    read(fd, buffer, sizeof(buffer));
    printf("Получено: %s\n", buffer);
    close(fd);
    return 0;
}