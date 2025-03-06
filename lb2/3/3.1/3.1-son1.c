#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  printf("Son1: Started (PID: %d)\n", getpid());
  while (1) {
  }
  return 0;
}