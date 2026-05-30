#include <proc.h>
#include <memory.h>

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
  char *const argv[] = { "--skip" };
  context_uload(&pcb[1], "/bin/pal", argv, (char *const []){ NULL });
  // context_uload(&pcb[1], "/bin/exec-test", (char *const []){ NULL }, (char *const []){ NULL   });
  
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");

}

void context_kload(PCB* n_pcb, void (*entry)(void *), void *arg) {
  n_pcb->cp = kcontext((Area) { n_pcb->stack, n_pcb + 1 }, entry, arg);
}

size_t context_uload(PCB* n_pcb, const char* filename, char *const argv[], char *const envp[]) {
  protect(&(n_pcb->as));
  void *new_stack = new_page(STACK_SIZE / PGSIZE);
  uintptr_t usp = (uintptr_t)new_stack + STACK_SIZE;

  uintptr_t new_stack_va = (uintptr_t)(n_pcb -> as.area.end) - STACK_SIZE;
  uintptr_t usp_va = (uintptr_t)(new_stack_va + STACK_SIZE);

  for (int i=0; i<STACK_SIZE/PGSIZE; i++) {
    map(&(n_pcb->as), (char*)(new_stack_va+ PGSIZE*i), (char*)(new_stack+ PGSIZE*i), 0b111);
  }
  

  int n_arg = 0, n_env = 0;

  if (envp != NULL) {
    for (; envp[n_env] != NULL; n_env++);
  }

  if (argv != NULL) {
    for (; argv[n_arg] != NULL; n_arg++); 
  }

  n_arg++;
  uintptr_t arg_ptr[n_arg], env_ptr[n_env];


  usp -= sizeof(uintptr_t); 
  *((uintptr_t*)usp) = 0;
  usp_va -= sizeof(uintptr_t);

  if (envp != NULL) {
    for (int i=n_env-1; i>=0; i--) {
      usp -= strlen(envp[i])+1;
      usp_va -= strlen(envp[i])+1;
      memcpy((char*)usp, envp[i], strlen(envp[i])+1);
      env_ptr[i] = usp_va;
    }
  }

  if (argv != NULL) {
    for (int i = n_arg-2; i>=0; i--) {
      usp -= strlen(argv[i]) + 1;
      usp_va -= strlen(argv[i])+1;
      memcpy((char*)usp, argv[i], strlen(argv[i])+1);
      arg_ptr[i+1] = usp_va;
    }
  }

  usp -= strlen(filename)+1;
  usp_va -= strlen(filename)+1;
  memcpy((char*)usp, filename, strlen(filename)+1);
  arg_ptr[0] = usp_va;


  usp -= sizeof(uintptr_t); 
  usp_va -= sizeof(uintptr_t);
  *((uintptr_t*)usp) = 0;

  if (n_env >= 0) {
    usp -= sizeof(env_ptr);
    usp_va -= sizeof(env_ptr);
    memcpy((char*)usp, env_ptr, sizeof(env_ptr));
  }

  usp -= sizeof(uintptr_t); 
  usp_va -= sizeof(uintptr_t);
  *((uintptr_t*)usp) = 0;

  if (n_arg >= 0) {
    usp -= sizeof(arg_ptr);
    usp_va -= sizeof(arg_ptr);
    memcpy((char*)usp, arg_ptr, sizeof(arg_ptr));
  }

  uintptr_t entry = loader(n_pcb, filename);

  n_pcb->cp = ucontext(&(n_pcb->as), (Area) { (void*)&(n_pcb->stack[0]), (void*)((uintptr_t)&(n_pcb->stack[0]) + STACK_SIZE) }, (void*)entry, usp);

  usp -= sizeof(uintptr_t);
  usp_va -= sizeof(uintptr_t);
  *((uintptr_t*)usp) = n_arg;
  (n_pcb->cp)->GPRx = usp_va;

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
