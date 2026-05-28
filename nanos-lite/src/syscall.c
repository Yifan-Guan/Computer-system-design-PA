#include <common.h>
#include "syscall.h"

extern char end;
uintptr_t program_break = (uintptr_t)&end;

void do_syscall(Context *c) {

  printf("[SYSCALL] syscall ID = %d\n at %p", c->GPR1, (void *)c->mepc);

  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit: halt(a[1]); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_write: 
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
    case SYS_brk:
      if (a[1] == 0) {
        c->GPRx = program_break;
      } else {
        uintptr_t new_break = a[1];
        if (new_break > program_break) {
          memset((void *)program_break, 0, new_break - program_break);
        }
        program_break = new_break;
        c->GPRx = 0;
      }
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
