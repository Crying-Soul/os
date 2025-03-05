#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execle() {
    setenv("MY_VAR", "HelloFrom execle", 1);
    printf("\n=== execle() ===\n");
    execle("/bin/echo", "echo", "From execle:", getenv("MY_VAR"), (char *)NULL, (char *)NULL);
    printf("After execle\n");
}

int main() {
    test_execle();
    return 0;
}
