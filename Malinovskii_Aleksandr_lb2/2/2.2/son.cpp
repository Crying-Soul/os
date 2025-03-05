#include <iostream>
#include <unistd.h>

int main()
{
    int pid, ppid;
    pid = getpid();
    ppid = getppid();

    std::cout << "SON PARAMS: pid=" << pid << " ppid=" << ppid << '\n';
    system("ps -H f | head -n 7 > log_son.txt");
    std::cout << "son process is finished"  << '\n';
    return 0;
}
