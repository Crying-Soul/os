#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execl() {
    setenv("MY_VAR", "HelloWorld from execl", 1);
    printf("\n=== execl() ===\n");
    printf("Before execl\n");
    execl("/bin/echo", "echo", "My environment variable:", getenv("MY_VAR"), (char *)NULL);
    printf("After execl\n");
}

int main() {
    test_execl();
    return 0;
}
