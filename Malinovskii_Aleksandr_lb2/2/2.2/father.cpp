#include <iostream>
#include <wait.h>
#include <unistd.h>

int main()
{
    int pid, ppid, status;

    pid = getpid();
    ppid = getppid();
    std::cout << "FATHER PARAM: pid=" << pid << " ppid=" << ppid << "\n";

    if (fork() == 0){
        execl("son", "son", NULL);
    }
    wait(&status);
    system("ps -H f | head -n 6 > log_father.txt");
    std::cout << "father process is finished with status: " << status << '\n';
    return 0;
}
