#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execve() {
    setenv("MY_VAR", "HelloFrom execve", 1);
    printf("\n=== execve() ===\n");
    char *args[] = {"echo", "Execve", getenv("MY_VAR"), NULL};
    char *env[] = {"MY_VAR=HelloFrom execve", NULL};
    printf("Before execve\n");
    execve("/bin/echo", args, env);
    printf("After execve\n");
}

int main() {
    test_execve();
    return 0;
}
