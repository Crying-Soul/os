#include <stdio.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


int main()
{
    int i, ppid, pid[6], status[5], result[5];
    char *son[] = {"son1", "son2", "son3", "son4", "son5"};
    int option[] = {WNOHANG, WUNTRACED, WNOHANG, WNOHANG, WNOHANG};

    
    pid[5] = getpid();
    ppid = getppid();
    printf("Father parameters was: pid=%i ppid=%i\n", pid[5],ppid);
    for (i = 0; i < 5; i++)
    {
        if((pid[i] = fork()) == 0)
        {
            execl(son[i], son[i], NULL);
        }
    }

    printf("Father: Sending SIGTERM(15) to son3 (pid=%d)\n", pid[2]);
    kill(pid[2], SIGTERM);
    printf("Father: Sending SIGKILL(9) to son3 (pid=%d)\n", pid[2]);
    kill(pid[4], SIGKILL);
    system("ps -H f > file.txt");
    for (i = 0; i < 5; i++)
    {
        result[i] = waitpid(pid[i], &status[i], option[i]);
        printf("%d) Child with pid = %d is finished with status %d\n", (1+i), result[i], status[i]);
    }

    for (i = 0; i < 5; i++) {
        if (WIFEXITED(status[i])) {
            printf("Process pid = %d exited with status %d\n", pid[i], WEXITSTATUS(status[i]));
        } else if (WIFSIGNALED(status[i])) {
            printf("Process pid = %d was terminated by signal %d\n", pid[i], WTERMSIG(status[i]));
        } else {
            printf("Process pid = %d is still running or failed\n", pid[i]);
        }
    }
    return 0;
}
