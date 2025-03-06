#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE 32
#define FIFO_PRIORITY 50

const char* get_scheduling_policy(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "UNKNOWN";
    }
}

void* thread_function(void* arg) {
    char timestamp[20];
    int policy;
    struct sched_param param = {.sched_priority = FIFO_PRIORITY};
    
    // Устанавливаем политику и приоритет для текущего потока
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
        return NULL;
    }
    
    int iteration = 0;
    while (iteration < 5) {
        pthread_getschedparam(pthread_self(), &policy, &param);
        printf("%-7s | PID: %-3d | TID: %-4ld | Priority: %-3d | Policy: %-12s\n",
               "Thread",  getpid(), syscall(SYS_gettid), 
               param.sched_priority, get_scheduling_policy(policy));
        fflush(stdout);
        iteration++;
    }
    return NULL;
}

int main() {
    struct sched_param main_param = {.sched_priority = FIFO_PRIORITY};
    
    // Устанавливаем политику и приоритет для основного потока
    if (sched_setscheduler(getpid(), SCHED_FIFO, &main_param) != 0) {
        perror("sched_setscheduler");
        return 1;
    }
    
    pthread_t thread1, thread2;
    
    // Создаем потоки
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    
    char timestamp[20];
    int policy;
    int iteration = 0;
    
    while (iteration < 5) {
        policy = sched_getscheduler(getpid());
        sched_getparam(getpid(), &main_param);
        printf("%-7s | PID: %-3d | TID: %-4ld | Priority: %-3d | Policy: %-12s\n",
               "Main", getpid(), syscall(SYS_gettid), 
               main_param.sched_priority, get_scheduling_policy(policy));
        fflush(stdout);
        iteration++;
    
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}