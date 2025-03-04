#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execv() {
    setenv("MY_VAR", "HelloFrom execv", 1);
    printf("\n=== execv() ===\n");
    char *args[] = {"echo", "Execv", getenv("MY_VAR"), NULL};
    printf("Before execv\n");
    execv("/bin/echo", args);
    printf("After execv\n");
}

int main() {
    test_execv();
    return 0;
}
