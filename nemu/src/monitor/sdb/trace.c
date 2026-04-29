#include "sdb.h"

#define RING_BUF_SIZE 16
typedef struct {
    word_t pc;
    uint32_t inst;
} RingBufEntry;

static RingBufEntry ring_buf[RING_BUF_SIZE];
static int ring_buf_index = 0;

void ring_buf_add(word_t pc, uint32_t inst) {
    ring_buf[ring_buf_index].pc = pc;
    ring_buf[ring_buf_index].inst = inst;
    ring_buf_index = (ring_buf_index + 1) % RING_BUF_SIZE;
}

void ring_buf_print() {
    printf("Ring Buffer (last %d instructions):\n", RING_BUF_SIZE);
    printf("PC \t\t Instruction\n");
    for (int i = 0; i < RING_BUF_SIZE; i++) {
        int idx = (ring_buf_index + i + 1) % RING_BUF_SIZE;
        if (ring_buf[idx].inst != 0) { // Only print valid entries
            printf("0x%08x \t 0x%08x\n", ring_buf[idx].pc, ring_buf[idx].inst);
        }
    }
}

#define MTRACE_BUF_SIZE 200

typedef struct {
    word_t addr;
    int is_vaddr;
    int is_write;
} MTraceEntry;

static MTraceEntry mtrace_buf[MTRACE_BUF_SIZE];
static int mtrace_index = 0;

void mtrace_add(word_t addr, int is_vaddr, int is_write) {
    mtrace_buf[mtrace_index].addr = addr;
    mtrace_buf[mtrace_index].is_vaddr = is_vaddr;
    mtrace_buf[mtrace_index].is_write = is_write;
    mtrace_index = (mtrace_index + 1) % MTRACE_BUF_SIZE;
}

void mtrace_print() {
    printf("Memory Trace (last %d accesses):\n", MTRACE_BUF_SIZE);
    printf("Address \t Type \t Access\n");
    for (int i = 0; i < MTRACE_BUF_SIZE; i++) {
        int idx = (mtrace_index + i + 1) % MTRACE_BUF_SIZE;
        if (mtrace_buf[idx].addr != 0) { // Only print valid entries
            printf("0x%08x \t %s \t %s\n", mtrace_buf[idx].addr,
                   mtrace_buf[idx].is_vaddr ? "vaddr" : "paddr",
                   mtrace_buf[idx].is_write ? "write" : "read");
        }
    }
}
