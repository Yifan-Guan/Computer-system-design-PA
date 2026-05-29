#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;

  const char *prefix = ev.keydown ? "kd " : "ku ";
  const char *name = keyname[ev.keycode];
  char *out = buf;
  size_t written = 0;

  for (const char *p = prefix; *p && written + 1 < len; p++) {
    out[written++] = *p;
  }

  for (const char *p = name; *p && written + 1 < len; p++) {
    out[written++] = *p;
  }

  if (written + 1 < len) {
    out[written++] = '\n';
  }

  if (len > 0) {
    out[written < len ? written : len - 1] = '\0';
  }

  return written;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
