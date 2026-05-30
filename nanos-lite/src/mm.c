#include <memory.h>
#include <proc.h>

static void *pf = NULL;
extern PCB *current;

void* new_page(size_t nr_page) {
  void* ret = pf;
  pf += nr_page * PGSIZE;

  return ret;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  void* new_pg = new_page(n/PGSIZE);
  memset(new_pg, 0, n);
  return new_pg;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  if (brk == 0) {
    return current->max_brk;
  }
  uintptr_t curbrk = current->max_brk;
  uintptr_t oldbrk = curbrk;
  if (brk > curbrk) {
    while (brk > curbrk) {
      map(&(current->as), (char*)curbrk, pg_alloc(PGSIZE), 0b111);
      curbrk += PGSIZE;
    }
    current->max_brk = curbrk;
  }
  return oldbrk;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
