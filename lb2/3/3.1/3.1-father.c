#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main()
{
    pid_t son1, son2, son3;
    system("> processes_after.txt");
    // Создаем son1
    son1 = fork();
    if (son1 == 0)
    {
        execl("./3.1-son1", "3.1-son1", NULL); // Запуск son1
        perror("execl son1 failed");
        exit(1);
    }
    sleep(1); // Даем время son1 запуститься
    printf("Father: Sending SIGINT to son1 (PID: %d)\n", son1);
    kill(son1, SIGINT); // Отправляем сигнал son1
    system("ps -s | tee processes_after.txt");
    waitpid(son1, NULL, 0); // Ждем завершения son1

    // Создаем son2
    son2 = fork();
    if (son2 == 0)
    {
        execl("./3.1-son2", "3.1-son2", NULL); // Запуск son2
        perror("execl son2 failed");
        exit(1);
    }
    sleep(1); // Даем время son2 запуститься
    printf("Father: Sending SIGINT to son2 (PID: %d)\n", son2);
    kill(son2, SIGINT); // Отправляем сигнал son2
    system("ps -s | tee processes_after.txt");
    waitpid(son2, NULL, 0); // Ждем завершения son2

    // Создаем son3
    son3 = fork();
    if (son3 == 0)
    {
        execl("./3.1-son3", "3.1-son3", NULL); // Запуск son3
        perror("execl son3 failed");
        exit(1);
    }
    sleep(1); // Даем время son3 запуститься
    printf("Father: Sending SIGINT to son3 (PID: %d)\n", son3);
    kill(son3, SIGINT); // Отправляем сигнал son3
    system("ps -s | tee processes_after.txt");
    waitpid(son3, NULL, 0); // Ждем завершения son3

    printf("Father: All children have exited.\n");
    return 0;
}