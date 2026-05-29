#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main() {
  struct timeval start;
  struct timezone current;
  gettimeofday(&start, NULL);
  printf("Start time: %d.%d seconds\n", start.tv_sec, start.tv_usec);
  while (1) {
    gettimeofday(&current, NULL);
    if (current.tv_sec * 1000 + current.tv_usec - start.tv_sec * 1000 - start.tv_usec >= 500) {
      printf("Current time: %d.%d seconds\n", current.tv_sec, current.tv_usec);
      break;
    }
  }
  return 0;
}
