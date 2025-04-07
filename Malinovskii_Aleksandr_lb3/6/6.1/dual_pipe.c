/******************************************************************************
 * 
 *  Цель:
 *      Продемонстрировать обмен данными между родительским и дочерним
 *      процессами с использованием анонимных каналов (pipes).
 *
 *  Описание:
 *      Программа создает два анонимных канала (pipe1 и pipe2). Родительский
 *      процесс отправляет сообщение дочернему процессу через pipe1, а дочерний
 *      процесс отправляет ответ родительскому процессу через pipe2.
 *
 *  Основные компоненты:
 *      - `pipe1[2]`:  Канал для передачи данных от родительского процесса к
 *         дочернему.
 *      - `pipe2[2]`:  Канал для передачи данных от дочернего процесса к
 *         родительскому.
 *      - `fork()`: Создает дочерний процесс.
 *      - `read()`: Читает данные из канала.
 *      - `write()`: Записывает данные в канал.
 *      - `close()`: Закрывает неиспользуемые концы каналов.
 *
 *  Схема работы:
 *      1. Родительский процесс создает два канала (pipe1 и pipe2).
 *      2. Родительский процесс создает дочерний процесс с помощью `fork()`.
 *      3. В родительском процессе:
 *         - Закрывается конец канала для чтения из pipe1 и конец канала для
 *           записи в pipe2.
 *         - Отправляется сообщение дочернему процессу через pipe1.
 *         - Ожидается получение ответа от дочернего процесса через pipe2.
 *         - Закрываются оставшиеся концы каналов и ожидается завершение
 *           дочернего процесса.
 *      4. В дочернем процессе:
 *         - Закрывается конец канала для записи в pipe1 и конец канала для
 *           чтения из pipe2.
 *         - Считывается сообщение от родительского процесса из pipe1.
 *         - Отправляется ответ родительскому процессу через pipe2.
 *         - Закрываются оставшиеся концы каналов.
 *
 *  Примечания:
 *      - Анонимные каналы используются для обмена данными между процессами,
 *        имеющими общего предка.
 *      - Важно закрывать неиспользуемые концы каналов, чтобы избежать
 *        утечек файловых дескрипторов.
 *      - Для надежной передачи данных следует проверять возвращаемые значения
 *        функций `read()` и `write()`.
 *
 ******************************************************************************/


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