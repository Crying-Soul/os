#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int signum) {
    printf("\nПерехвачен SIGINT (Ctrl+C)\n");
    signal(SIGINT, SIG_DFL);
}

int main() {
    signal(SIGINT, sigint_handler);
    printf("Нажмите Ctrl+C (1 раз — перехват, 2 раз — выход)\n");
    while (1) pause();
    return 0;
}
