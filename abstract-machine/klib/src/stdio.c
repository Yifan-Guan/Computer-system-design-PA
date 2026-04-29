#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// static char* int_to_str(int num, char* buf_end) {
//   char* p = buf_end;
//   int negative = num < 0;
//   if (negative) {
//     num = -num;
//   }

//   int n = num;
//   *p-- = '\0';
//   for(; n >= 10; n /= 10) *p-- = '0' + (n % 10);
//   if (n) *p = '0' + n;
//   if (negative) *--p = '-';

//   return p;
// }

int printf(const char *fmt, ...) {
  char buf[1024];
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsprintf(buf, fmt, ap);
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

// int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
//   if (n == 0) return 0;
//   char *a;
//   char *s = (char*)fmt;
//   int len = 0;

//   const int num_buf_size = 20;
//   char num_buf[num_buf_size];
//   char* num_buf_end = num_buf + num_buf_size - 1;

//   for(;;) {
//     if(!*s || len >= n - 1) break;

//     for(a=s; *s && *s != '%'; s++);
//     int l = s - a;
//     if (l) {
//       if (len + l >= n) {
//         l = n - len - 1;
//       }
//       strncpy(out + len, a, l);
//       len += l;
//     }
//     s++;

//     switch (*s) {
//       case 'd':
//         int i = va_arg(ap, int);
//         a = int_to_str(i, num_buf_end);
//         l = strlen(a);
//         break;
//       case 's':
//         a = va_arg(ap, char*);
//         l = strlen(a);
//         if (a == NULL) a = "(null)";
//         break;
//       default:
//         assert(0);
//     }

//     if (len + l >= n) {
//       l = n - len - 1;
//     }
//     strncpy(out + len, a, l);
//     len += l;

//     s++;
//   }
//   out[len] = '\0';

//   return len;
// }

#endif
