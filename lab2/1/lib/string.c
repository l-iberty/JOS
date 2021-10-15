#include <include/types.h>

int strlen(const char *s) {
  int n = 0;
  while (*s++ != '\0') {
    n++;
  }
  return n;
}

char *strcpy(char *dst, const char *src) {
  char *r = dst;
  while ((*dst++ = *src++) != '\0')
    /* do nothing */ ;
  return r;
}

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q) p++, q++;
  return (int)((unsigned char)*p - (unsigned char)*q);
}

int strncmp(const char *p, const char *q, size_t n) {
  while (n > 0 && *p && *p == *q) n--, p++, q++;
  if (n == 0) {
    return 0;
  } else {
    return (int)((unsigned char)*p - (unsigned char)*q);
  }
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a null pointer if the string has no 'c'.
char *strchr(const char *s, char c) {
  for (; *s; s++) {
    if (*s == c) return (char *)s;
  }
  return 0;
}

void memset(void *dst, char ch, int n) {
  while (n-- > 0) {
    *(char *)dst++ = ch;
  }
}

void *memmove(void *dst, const void *src, size_t n) {
  const char *s;
  char *d;

  s = src;
  d = dst;
  if (s < d && s + n > d) {
    s += n;
    d += n;
    while (n-- > 0) *--d = *--s;
  } else {
    while (n-- > 0) *d++ = *s++;
  }

  return dst;
}