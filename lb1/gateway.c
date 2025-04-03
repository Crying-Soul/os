#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    // Открываем файл
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Open file error");
        exit(1);
    }

    // Читаем и выводим содержимое файла
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        if (bytesRead == -1) {
            perror("Read file error");
            close(fd);
            exit(1);
        }
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    // Проверяем, завершилось ли чтение с ошибкой
    if (bytesRead == -1) {
        perror("Read file error");
        close(fd);
        exit(1);
    }

    close(fd);
    return 0;
}