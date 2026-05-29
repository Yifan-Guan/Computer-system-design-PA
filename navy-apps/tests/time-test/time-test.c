#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int main() {
  struct timeval start;
  struct timeval current;
  gettimeofday(&start, NULL);
  printf("Start\n");
  while (1) {
    gettimeofday(&current, NULL);
    if (current.tv_sec * 1000 + current.tv_usec - start.tv_sec * 1000 - start.tv_usec >= 500) {
      printf("Tick\n");
      start.tv_sec = current.tv_sec;
      start.tv_usec = current.tv_usec;
    }
  }
  return 0;
}
