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
    long long start_us = start.tv_sec * 1000000 + start.tv_usec;
    long long current_us = current.tv_sec * 1000000 + current.tv_usec;
    if (current_us - start_us >= 500000) {
      printf("Tick\n");
      start.tv_sec = current.tv_sec;
      start.tv_usec = current.tv_usec;
    }
  }
  return 0;
}
