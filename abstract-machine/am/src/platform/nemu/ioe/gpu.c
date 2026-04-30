#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
   int i;
   int w = 400;  // TODO: get the correct width
   int h = 300;  // TODO: get the correct height
   uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
   for (i = 0; i < w * h; i ++) fb[i] = i;
   outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 400, .height = 300,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int wi = 400;
  int w = ctl->w, h = ctl->h;
  int x = ctl->x, y = ctl->y;
  uint32_t *pi = (uint32_t *)(ctl->pixels);

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      fb[(y + i) * wi + x + j] = pi[i * w + j];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

void __am_gpu_memcpy(AM_GPU_MEMCPY_T *ctl) {
  uint32_t *dest = (uint32_t *)(ctl->dest);
  uint32_t *src = (uint32_t *)(ctl->src);
  for (int i = 0; i < ctl->size / sizeof(uint32_t); i++) {
    dest[i] = src[i];
  }
}
