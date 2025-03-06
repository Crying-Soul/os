#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void test_file_inheritance(void)
{
    printf("=== Тест 6.1: Наследование файловых дескрипторов ===\n");
    fflush(stdout);

    int fd = open("out.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    const char *parent_line1 = "Parent writes line 1\n";
    write(fd, parent_line1, strlen(parent_line1));

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        const char *child_line = "Child writes line 2\n";
        write(fd, child_line, strlen(child_line));
        printf("Child (PID %d): Записал данные в 'out.txt'\n", getpid());
        fflush(stdout);
        close(fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        wait(NULL);
        const char *parent_line2 = "Parent writes line 3\n";
        write(fd, parent_line2, strlen(parent_line2));
        printf("Parent (PID %d): Записал данные в 'out.txt'\n", getpid());
        fflush(stdout);
        close(fd);
    }

    printf("Проверьте содержимое файла 'out.txt'.\n\n");
    fflush(stdout);
}

int main()
{
    test_file_inheritance();
    return 0;
}
