#include <common.h>
#include <fs.h>
#include <time.h>
#include "syscall.h"

extern char end;
static uintptr_t program_break = (uintptr_t)&end;

void do_syscall(Context *c) {

  // Log("[SYSCALL] syscall ID = %d\n at %p", c->GPR1, (void *)c->mepc);

  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_exit: halt(a[1]); break;
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_open: c->GPRx = fs_open((const char *)a[1], a[2], a[3]); break;
    case SYS_read: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: c->GPRx = fs_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
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
    case SYS_gettimeofday: 
      uint32_t tick = io_read(AM_TIMER_UPTIME).us;
      ((struct timeval *)a[1])->tv_sec = tick / 1000000;
      ((struct timeval *)a[1])->tv_usec = tick % 1000000;
      c->GPRx = 0;
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
