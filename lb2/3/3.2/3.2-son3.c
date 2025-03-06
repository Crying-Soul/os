#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSon parameters: pid=%i, ppid=%i\n", pid, ppid);

  exit(512); // son3: exit(-1)
}
