#define _GNU_SOURCE // Для clone()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define STACK_SIZE (1024 * 1024)

// Функция, выполняемая clone()-потомком
int clone_func(void *arg)
{
    int fd = *(int *)arg;
    printf("Clone-потомок (PID %d) запущен. Используем файловый дескриптор: %d\n", getpid(), fd);
    fflush(stdout);

    // Записываем сообщение в файл
    const char *clone_line = "Clone child writes line\n";
    ssize_t bytes_written = write(fd, clone_line, strlen(clone_line));
    if (bytes_written < 0)
    {
        perror("write in clone_func");
        return -1;
    }

    // Получаем и выводим текущее смещение файла, демонстрируя, что оно разделяется с родителем
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset == (off_t)-1)
    {
        perror("lseek in clone_func");
    }
    else
    {
        printf("Clone-потомок (PID %d): текущее смещение файла = %ld\n", getpid(), (long)offset);
    }
    fflush(stdout);

    return 0;
}

void test_clone_inheritance(void)
{
    printf("=== Тест 6.4: Наследование в clone() с CLONE_FILES ===\n");
    fflush(stdout);

    // Открываем файл без O_APPEND, чтобы можно было проследить за изменением смещения
    int fd = open("out.txt", O_CREAT | O_WRONLY, 0644);
    if (fd < 0)
    {
        perror("open (clone test)");
        exit(EXIT_FAILURE);
    }

    // Перемещаемся в конец файла, если он уже существует
    if (lseek(fd, 0, SEEK_END) == (off_t)-1)
    {
        perror("lseek перед записью родителя");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Родитель записывает строку до вызова clone()
    const char *parent_before = "Parent (before clone) writes line\n";
    ssize_t bytes_written = write(fd, parent_before, strlen(parent_before));
    if (bytes_written < 0)
    {
        perror("write parent_before");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Выводим смещение файла после записи родителя
    off_t parent_offset_before = lseek(fd, 0, SEEK_CUR);
    if (parent_offset_before == (off_t)-1)
    {
        perror("lseek после записи родителя");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Родитель (PID %d): смещение файла до clone = %ld\n", getpid(), (long)parent_offset_before);
    fflush(stdout);

    // Выделяем память для стека clone()-потомка
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    // Флаги clone: SIGCHLD (для уведомления о завершении) и CLONE_FILES (разделение файловых дескрипторов)
    int clone_flags = SIGCHLD | CLONE_FILES;
    pid_t child_pid = clone(clone_func, stack_top, clone_flags, &fd);
    if (child_pid == -1)
    {
        perror("clone");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Ожидаем завершения clone()-потомка
    if (waitpid(child_pid, NULL, 0) == -1)
    {
        perror("waitpid");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Родитель получает смещение файла после работы clone()-потомка
    off_t parent_offset_after = lseek(fd, 0, SEEK_CUR);
    if (parent_offset_after == (off_t)-1)
    {
        perror("lseek после clone");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Родитель (PID %d): смещение файла после clone = %ld\n", getpid(), (long)parent_offset_after);
    fflush(stdout);

    // Родитель записывает строку после завершения clone()-потомка
    const char *parent_after = "Parent (after clone) writes line\n";
    bytes_written = write(fd, parent_after, strlen(parent_after));
    if (bytes_written < 0)
    {
        perror("write parent_after");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    free(stack);

    printf("Завершён тест clone наследования. Проверьте содержимое файла 'out.txt'.\n\n");
    fflush(stdout);
}

int main()
{
    test_clone_inheritance();
    return 0;
}
