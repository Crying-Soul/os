#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
int main() {
    int pid, ppid;
    pid = getpid();
    ppid = getppid();

    printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);
    int j=0;
    for (int i=0;i<10000000;i++){
        j++;
    }
    exit(-1); // son3: exit(-1)
}
