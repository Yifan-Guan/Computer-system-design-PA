#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char* int_to_str(int num, char* buf_end) {
  char* p = buf_end;
  int negative = num < 0;
  if (negative) {
    num = -num;
  }

  int n = num;
  *p-- = '\0';
  for(; n > 10; n /= 10) *p-- = '0' + (n % 10);
  if (n) *p = '0' + n;
  if (negative) *--p = '-';

  return p;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
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

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  char *a;
  char *s = (char*)fmt;
  int len = 0;

  const int num_buf_size = 20;
  char num_buf[num_buf_size];
  char* num_buf_end = num_buf + num_buf_size - 1;

  for(;;) {
    if(!*s || len >= n - 1) break;

    for(a=s; *s && *s != '%'; s++);
    int l = s - a;
    if (l) {
      if (len + l >= n) {
        l = n - len - 1;
      }
      strncpy(out + len, a, l);
      len += l;
    }
    s++;

    switch (*s) {
      case 'd':
        int i = va_arg(ap, int);
        a = int_to_str(i, num_buf_end);
        l = num_buf_end - a;
        a = "test";
        l = 4;
        break;
      case 's':
        a = va_arg(ap, char*);
        l = strlen(a);
        if (a == NULL) a = "(null)";
        break;
      default:
        assert(0);
    }

    if (len + l >= n) {
      l = n - len - 1;
    }
    strncpy(out + len, a, l);
    len += l;

    s++;
  }
  out[len] = '\0';

  return len;
}

#endif
