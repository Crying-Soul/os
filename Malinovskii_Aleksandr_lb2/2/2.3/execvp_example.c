#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execvp() {
    setenv("MY_VAR", "HelloFrom execvp", 1);
    printf("\n=== execvp() ===\n");
    char *args[] = {"echo", "Execvp", getenv("MY_VAR"), NULL};
    printf("Before execvp\n");
    execvp("echo", args);
    printf("After execvp\n");
}

int main() {
    test_execvp();
    return 0;
}
