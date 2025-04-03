#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
int main() {
    int pipefd[2];
    pid_t pid;
    
    if (pipe(pipefd)) {
        perror("pipe");
        return 1;
    }
    
    pid = fork();
    if (pid == 0) { // Дочерний процесс
        close(pipefd[1]); // Закрываем запись
        
        // Перенаправляем stdin на чтение из pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        // Запускаем команду, которая читает из stdin
        execlp("wc", "wc", "-l", NULL);
        perror("execlp");
        return 1;
    } else { // Родительский процесс
        close(pipefd[0]); // Закрываем чтение
        
        // Пишем в pipe строки, которые будет считать wc
        const char* lines[] = {"Первая строка\n", "Вторая строка\n", "Третья строка\n"};
        for (int i = 0; i < 3; i++) {
            write(pipefd[1], lines[i], strlen(lines[i]));
        }
        
        close(pipefd[1]);
        wait(NULL); // Ждем завершения дочернего процесса
    }
    
    return 0;
}