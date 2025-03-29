#include <stdio.h>
#include <sys/stat.h>

int main() {
    struct stat sb;
    stat("cmd_pipe", &sb);
    
    printf("Тип: %s\n", S_ISFIFO(sb.st_mode) ? "FIFO" : "Не FIFO");
    printf("Inode: %lu\n", sb.st_ino);
    printf("Права: %o\n", sb.st_mode & 0777);
    printf("Размер: %ld\n", sb.st_size);
    
    return 0;
}