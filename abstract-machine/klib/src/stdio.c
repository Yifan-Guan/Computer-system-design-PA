#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)



int printf(const char *fmt, ...) {
  char buf[4096];
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  for (char *p = buf; *p; p++) {
    putch(*p);
  }
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, 0x7fffffff, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

static char* int_to_str(int num, char* buf_end) {
  char *p = buf_end;
  int neg = num < 0;
  if (neg) num = -num;

  *p-- = '\0';

  do {
    *p-- = '0' + (num % 10);
    num /= 10;
  } while (num > 0);

  if (neg) *p-- = '-';

  return p + 1;
}

static char* ptr_to_str(uintptr_t ptr, char* buf_end) {
  char *p = buf_end;

  *p-- = '\0';

  do {
    int digit = ptr % 16;
    if (digit < 10) {
      *p-- = '0' + digit;
    } else {
      *p-- = 'a' + (digit - 10);
    }
    ptr /= 16;
  } while (ptr > 0);

  *p-- = 'x';
  *p-- = '0';

  return p + 1;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t len = 0;   // number of chars actually written
  size_t total = 0; // total chars that would be written

  if (n == 0) out = NULL; // prevent writing

  const int BUF_SIZE = 32;
  char num_buf[BUF_SIZE];

  for (const char *p = fmt; *p; p++) {
    if (*p != '%') {
      // normal character
      if (len + 1 < n) out[len] = *p;
      len += (len + 1 < n);
      total++;
      continue;
    }

    // handle format
    p++; // skip '%'

    char *str = NULL;
    int l = 0;

    if (*p == 'd') {
      int val = va_arg(ap, int);
      str = int_to_str(val, num_buf + BUF_SIZE - 1);
      l = strlen(str);
    } else if (*p == 's') {
      str = va_arg(ap, char*);
      if (str == NULL) str = "(null)";
      l = strlen(str);
    } else if (*p == '%') {
      str = "%";
      l = 1;
    } else if (*p == 'p') {
      uintptr_t val = (uintptr_t)va_arg(ap, void*);
      str = ptr_to_str(val, num_buf + BUF_SIZE - 1);
      l = strlen(str);
    } else {
      // unsupported, just print it literally
      if (len + 1 < n) out[len] = *p;
      len += (len + 1 < n);
      total++;
      continue;
    }

    // copy string
    for (int i = 0; i < l; i++) {
      if (len + 1 < n) out[len] = str[i];
      len += (len + 1 < n);
      total++;
    }
  }

  // null terminate
  if (n > 0) {
    if (len < n) out[len] = '\0';
    else out[n - 1] = '\0';
  }

  return total;
}

#endif
