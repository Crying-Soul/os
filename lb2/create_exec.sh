#!/bin/bash

rm -rf exec_examples
# Создаем папку для файлов
mkdir -p exec_examples

# Переходим в созданную папку
cd exec_examples

# Создаем файл для execl
cat > execl_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execl() {
    setenv("MY_VAR", "HelloWorld from execl", 1);
    printf("\\n=== execl() ===\\n");
    printf("Before execl\\n");
    execl("/bin/echo", "echo", "My environment variable:", getenv("MY_VAR"), (char *)NULL);
    printf("After execl\\n");
}

int main() {
    test_execl();
    return 0;
}
EOL

# Создаем файл для execlp
cat > execlp_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execlp() {
    setenv("MY_VAR", "HelloFrom execlp", 1);
    printf("\\n=== execlp() ===\\n");
    printf("Before execlp\\n");
    execlp("echo", "echo", "From execlp:", getenv("MY_VAR"), (char *)NULL);
    printf("After execlp\\n");
}

int main() {
    test_execlp();
    return 0;
}
EOL

# Создаем файл для execle
cat > execle_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execle() {
    setenv("MY_VAR", "HelloFrom execle", 1);
    printf("\\n=== execle() ===\\n");
    execle("/bin/echo", "echo", "From execle:", getenv("MY_VAR"), (char *)NULL, (char *)NULL);
    printf("After execle\\n");
}

int main() {
    test_execle();
    return 0;
}
EOL

# Создаем файл для execv
cat > execv_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execv() {
    setenv("MY_VAR", "HelloFrom execv", 1);
    printf("\\n=== execv() ===\\n");
    char *args[] = {"echo", "Execv", getenv("MY_VAR"), NULL};
    printf("Before execv\\n");
    execv("/bin/echo", args);
    printf("After execv\\n");
}

int main() {
    test_execv();
    return 0;
}
EOL

# Создаем файл для execvp
cat > execvp_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execvp() {
    setenv("MY_VAR", "HelloFrom execvp", 1);
    printf("\\n=== execvp() ===\\n");
    char *args[] = {"echo", "Execvp", getenv("MY_VAR"), NULL};
    printf("Before execvp\\n");
    execvp("echo", args);
    printf("After execvp\\n");
}

int main() {
    test_execvp();
    return 0;
}
EOL

# Создаем файл для execve
cat > execve_example.c <<EOL
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_execve() {
    setenv("MY_VAR", "HelloFrom execve", 1);
    printf("\\n=== execve() ===\\n");
    char *args[] = {"echo", "Execve", getenv("MY_VAR"), NULL};
    char *env[] = {"MY_VAR=HelloFrom execve", NULL};
    printf("Before execve\\n");
    execve("/bin/echo", args, env);
    printf("After execve\\n");
}

int main() {
    test_execve();
    return 0;
}
EOL
