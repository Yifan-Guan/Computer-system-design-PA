#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

uintptr_t loader(PCB *pcb, const char *filename);
void naive_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal", NULL, NULL);
  
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");

}

void context_kload(PCB* n_pcb, void (*entry)(void *), void *arg) {
  n_pcb->cp = kcontext((Area) { n_pcb->stack, n_pcb + 1 }, entry, arg);
}

size_t context_uload(PCB* n_pcb, const char* filename, char *const argv[], char *const envp[]) {
  uintptr_t entry = loader(n_pcb, filename);
  n_pcb->cp = ucontext(&n_pcb->as, (Area) { n_pcb->stack, n_pcb + 1 }, (void*)entry);
  return 0;
}

Context* schedule(Context *prev) {
  current->cp = prev;
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (pcb[i].cp != NULL && pcb[i].cp != prev) {
      current = &pcb[i];
      return current->cp;
    }
  }
  return prev;
}
