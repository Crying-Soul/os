#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int counter = 0;
const int MAX_SIGNALS = 3;

void sigint_handler(int signum) {
    printf("\nПерехвачен SIGINT (Ctrl+C), раз: %d\n", ++counter);
    if (counter >= MAX_SIGNALS) {
        printf("Восстановлен стандартный обработчик\n");
        signal(SIGINT, SIG_DFL);
    }
}

int main() {
    signal(SIGINT, sigint_handler);
    printf("Нажмите Ctrl+C (3 раза — потом выход)\n");
    while (1) pause();
    return 0;
}
