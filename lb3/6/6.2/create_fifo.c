#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_NAME "prog_pipe"
#define FIFO_PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) // 0666

int main() {
    // Проверяем, не существует ли уже FIFO
    struct stat st;
    if (stat(FIFO_NAME, &st) == 0) {
        if (S_ISFIFO(st.st_mode)) {
            fprintf(stderr, "FIFO '%s' уже существует\n", FIFO_NAME);
        } else {
            fprintf(stderr, "Файл '%s' уже существует, но это не FIFO\n", FIFO_NAME);
        }
        return EXIT_FAILURE;
    }

    // Создаем FIFO
    if (mkfifo(FIFO_NAME, FIFO_PERMISSIONS) == -1) {
        perror("Ошибка создания FIFO");
        return EXIT_FAILURE;
    }

    printf("FIFO '%s' успешно создан с правами %o\n", FIFO_NAME, FIFO_PERMISSIONS);
    return EXIT_SUCCESS;
}