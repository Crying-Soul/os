#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipefd[2];
    pid_t pid;
    char buf[256];
    
    // Создаем pipe
    if (pipe(pipefd)) {
        perror("pipe");
        return 1;
    }
    
    pid = fork();
    if (pid == 0) { // Дочерний процесс
        close(pipefd[1]); // Закрываем запись
        
        // Читаем из pipe
        read(pipefd[0], buf, sizeof(buf));
        printf("Дочерний процесс (PID=%d) получил: '%s'\n", getpid(), buf);
        
        close(pipefd[0]);
    } else { // Родительский процесс
        close(pipefd[0]); // Закрываем чтение
        
        // Пишем в pipe
        const char* msg = "Привет из родительского процесса!";
        printf("Родительский процесс (PID=%d) отправляет: '%s'\n", getpid(), msg);
        write(pipefd[1], msg, strlen(msg) + 1);
        
        close(pipefd[1]);
        wait(NULL); // Ждем завершения дочернего процесса
    }
    
    return 0;
}