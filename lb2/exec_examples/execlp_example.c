#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execlp() {
    setenv("MY_VAR", "HelloFrom execlp", 1);
    printf("\n=== execlp() ===\n");
    printf("Before execlp\n");
    execlp("echo", "echo", "From execlp:", getenv("MY_VAR"), (char *)NULL);
    printf("After execlp\n");
}

int main() {
    test_execlp();
    return 0;
}
