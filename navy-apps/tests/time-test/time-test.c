#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  printf("Start\n");
  uint32_t last, current;
  last = NDL_GetTicks();
  while (1) {
    current = NDL_GetTicks();
    if (current - last >= 500) {
      printf("Current time: %u\n", current);
      last = current;
    }
  }
  NDL_Quit();
  return 0;
}
