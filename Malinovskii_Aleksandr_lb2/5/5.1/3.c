// program1.c
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sched.h>

int main() {
    int count = 0;
    while(1) {
        for(int i = 0; i < 10000; i++) count++; // Имитация работы
        printf("3: PID=%d, nice=%d, policy=%d\n", 
               getpid(), 
               getpriority(PRIO_PROCESS, 0),
               sched_getscheduler(0));
        fflush(stdout);
    }
    return 0;
}
