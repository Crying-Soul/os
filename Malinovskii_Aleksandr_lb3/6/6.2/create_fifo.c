#include <stdio.h>
#include <sys/stat.h>

int main() {
    if (mkfifo("prog_pipe", 0666) == -1) {
        perror("Ошибка создания FIFO");
        return 1;
    }
    printf("FIFO 'prog_pipe' успешно создан программно\n");
    return 0;
}