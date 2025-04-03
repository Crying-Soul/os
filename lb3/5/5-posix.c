#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define SHM_NAME "/my_shm"
#define SEM_WRITE "/sem_write"
#define SEM_READ  "/sem_read"
#define SHM_SIZE 1024

typedef struct {
    char data[SHM_SIZE];
} shared_data;

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <number_of_iterations>\n", prog_name);
    fprintf(stderr, "Example: %s 5\n", prog_name);
}

void cleanup_resources(shared_data *shdata, sem_t *sem_write, sem_t *sem_read) {
    if (sem_write != SEM_FAILED) {
        sem_close(sem_write);
        sem_unlink(SEM_WRITE);
    }
    if (sem_read != SEM_FAILED) {
        sem_close(sem_read);
        sem_unlink(SEM_READ);
    }
    if (shdata != MAP_FAILED) {
        munmap(shdata, sizeof(shared_data));
    }
    shm_unlink(SHM_NAME);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "Number of iterations must be a positive integer\n"); fflush(stdout);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Cleanup any leftover resources from previous runs
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_WRITE);
    sem_unlink(SEM_READ);

    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed"); fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate failed"); fflush(stdout);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    shared_data *shdata = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shdata == MAP_FAILED) {
        perror("mmap failed"); fflush(stdout);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    close(shm_fd); // File descriptor no longer needed

    // Create semaphores
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT, 0666, 1);
    if (sem_write == SEM_FAILED) {
        perror("sem_open (sem_write) failed"); fflush(stdout);
        cleanup_resources(shdata, SEM_FAILED, SEM_FAILED);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_read = sem_open(SEM_READ, O_CREAT, 0666, 0);
    if (sem_read == SEM_FAILED) {
        perror("sem_open (sem_read) failed"); fflush(stdout);
        cleanup_resources(shdata, sem_write, SEM_FAILED);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed"); fflush(stdout);
        cleanup_resources(shdata, sem_write, sem_read);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        // Reader process (child)
        printf("Reader process (PID: %d) started\n", getpid()); fflush(stdout);
        
        for (int i = 0; i < iterations; i++) {
            if (sem_wait(sem_read) == -1) {
                perror("sem_wait (sem_read) failed"); fflush(stdout);
                break;
            }
            
            printf("[Reader PID %d] Received: %s\n", getpid(), shdata->data); fflush(stdout);
            
            if (sem_post(sem_write) == -1) {
                perror("sem_post (sem_write) failed"); fflush(stdout);
                break;
            }
        }
        
        // Cleanup in child process
        sem_close(sem_write);
        sem_close(sem_read);
        munmap(shdata, sizeof(shared_data));
        exit(EXIT_SUCCESS);
    } else {
        // Writer process (parent)
        printf("Writer process (PID: %d) started, created reader (PID: %d)\n", getpid(), pid); fflush(stdout);
        
        for (int i = 0; i < iterations; i++) {
            if (sem_wait(sem_write) == -1) {
                perror("sem_wait (sem_write) failed"); fflush(stdout);
                break;
            }
            
            snprintf(shdata->data, SHM_SIZE, "Message %d from PID %d", i + 1, getpid()); fflush(stdout);
            printf("[Writer PID %d] Sent: %s\n", getpid(), shdata->data); fflush(stdout);
            
            if (sem_post(sem_read) == -1) {
                perror("sem_post (sem_read) failed"); fflush(stdout);
                break;
            }
        }
        
        // Wait for child to finish
        wait(NULL);
        
        // Cleanup resources
        cleanup_resources(shdata, sem_write, sem_read);
    }

    return EXIT_SUCCESS;
}