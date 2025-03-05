#!/bin/bash

# Создание son1.c
cat <<EOF > son1.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("\\nSon parameters: pid=%i, ppid=%i\\n", pid, ppid);

    return 0; // son1: return 0
}
EOF

# Создание son2.c
cat <<EOF > son2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("\\nSon parameters: pid=%i, ppid=%i\\n", pid, ppid);

    exit(64); // son2: exit(1)
}
EOF

# Создание son3.c
cat <<EOF > son3.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("\\nSon parameters: pid=%i, ppid=%i\\n", pid, ppid);

    exit(512); // son3: exit(-1)
}
EOF

# Создание son4.c
cat <<EOF > son4.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("\\nSon parameters: pid=%i, ppid=%i\\n", pid, ppid);

    return 5   ; // son4: return 5
}
EOF

# Создание son5.c
cat <<EOF > son5.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("\\nSon parameters: pid=%i, ppid=%i\\n", pid, ppid);

    exit(-10); 
}
EOF

echo "Файлы son1.c, son2.c, son3.c, son4.c, son5.c созданы."