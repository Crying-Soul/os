#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define PIPE_NAME "cmd_pipe"

void print_file_type(mode_t mode) {
    printf("Тип: ");
    if (S_ISFIFO(mode))      printf("FIFO (именованный канал)\n");
    else if (S_ISREG(mode))  printf("Обычный файл\n");
    else if (S_ISDIR(mode))  printf("Каталог\n");
    else if (S_ISLNK(mode))  printf("Символическая ссылка\n");
    else if (S_ISBLK(mode))  printf("Блочное устройство\n");
    else if (S_ISCHR(mode))  printf("Символьное устройство\n");
    else if (S_ISSOCK(mode)) printf("Сокет\n");
    else                     printf("Неизвестный тип файла\n");
}

void print_permissions(mode_t mode) {
    printf("Права доступа: %c%c%c %c%c%c %c%c%c (%04o)\n",
        (mode & S_IRUSR) ? 'r' : '-', (mode & S_IWUSR) ? 'w' : '-', (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-', (mode & S_IWGRP) ? 'w' : '-', (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-', (mode & S_IWOTH) ? 'w' : '-', (mode & S_IXOTH) ? 'x' : '-',
        mode & 07777);
}

int main() {
    struct stat sb;
    
    if (stat(PIPE_NAME, &sb) == -1) {
        fprintf(stderr, "Ошибка получения информации о файле '%s': %s\n", 
                PIPE_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Информация о файле: %s\n", PIPE_NAME);
    print_file_type(sb.st_mode);
    printf("Inode: %lu\n", (unsigned long)sb.st_ino);
    print_permissions(sb.st_mode);
    printf("Размер: %lld байт\n", (long long)sb.st_size);
    printf("Владелец: UID=%u, GID=%u\n", sb.st_uid, sb.st_gid);
    printf("Время последнего доступа: %s", ctime(&sb.st_atime));
    printf("Время последней модификации: %s", ctime(&sb.st_mtime));
    printf("Время последнего изменения статуса: %s", ctime(&sb.st_ctime));

    return EXIT_SUCCESS;
}