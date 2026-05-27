#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit: halt(a[1]); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_write: 
    Log("SYS_write: fd = %d, buf = %p, count = %d", a[1], (void *)a[2], a[3]);
      if (a[1] == 1 || a[1] == 2) {
        char *buf = (char *)a[2];
        size_t count = a[3];
        for (size_t i = 0; i < count; i++) {
          putch(buf[i]);
        }
        c->GPRx = count;
      } else {
        c->GPRx = -1;
      }
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
