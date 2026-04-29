#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char* p = s;
  while (*p) p++;
  return p - s;
}

char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  while ((*dst++ = *src++));
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;
  while (n && (*dst++ = *src++)) n--;
  memset(dst, 0, n);
  return ret;
}

char *strcat(char *dst, const char *src) {
  strcpy(dst + strlen(dst), src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  for(; *s1 && *s1 == *s2; s1++, s2++);
  return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const unsigned char *p1 = (void*)s1, *p2 = (void*)s2;
  if (!n--) return 0;
  for (; *p1 && *p2 && n && *p1 == *p2; p1++, p2++, n--);
  return *p1 - *p2;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  for(; n; n--, p++) *p = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char* d = dst;
  const char* s = src;

  if (d == s) return d;
  if ((uintptr_t)s + n <= (uintptr_t)d) return memcpy(d, s, n);

  if (d < s) {
    for (; n; n--) *d++ = *s++;
  } else {
    while(n) n--, d[n] = s[n];
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *d = out;
  const unsigned char *s = in;
  for (; n; n--) *d++ = *s++;
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = s1, *p2 = s2;
  for (; n && *p1 == *p2; n--, p1++, p2++);
  return n ? *p1 - *p2 : 0;
}

#endif
