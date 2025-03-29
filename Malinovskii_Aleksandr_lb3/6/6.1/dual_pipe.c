#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid;
    char buf[256];
    
    // Создаем два pipe
    if (pipe(pipe1) || pipe(pipe2)) {
        perror("pipe");
        return 1;
    }
    
    pid = fork();
    if (pid == 0) { // Дочерний процесс
        close(pipe1[1]); // Закрываем запись в первый pipe
        close(pipe2[0]); // Закрываем чтение из второго pipe
        
        // Читаем из первого pipe
        read(pipe1[0], buf, sizeof(buf));
        printf("Дочерний процесс (PID=%d) получил: '%s'\n", getpid(), buf);
        
        // Пишем во второй pipe
        const char* reply = "Ответ от дочернего процесса";
        printf("Дочерний процесс (PID=%d) отправляет: '%s'\n", getpid(), reply);
        write(pipe2[1], reply, strlen(reply) + 1);
        
        close(pipe1[0]);
        close(pipe2[1]);
    } else { // Родительский процесс
        close(pipe1[0]); // Закрываем чтение из первого pipe
        close(pipe2[1]); // Закрываем запись во второй pipe
        
        // Пишем в первый pipe
        const char* msg = "Сообщение от родительского процесса";
        printf("Родительский процесс (PID=%d) отправляет: '%s'\n", getpid(), msg);
        write(pipe1[1], msg, strlen(msg) + 1);
        
        // Читаем из второго pipe
        read(pipe2[0], buf, sizeof(buf));
        printf("Родительский процесс (PID=%d) получил: '%s'\n", getpid(), buf);
        
        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL); // Ждем завершения дочернего процесса
    }
    
    return 0;
}