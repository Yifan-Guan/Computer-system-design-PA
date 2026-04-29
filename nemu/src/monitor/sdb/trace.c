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