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

void memset(void *dst, char ch, int n) {
  while (n-- > 0) {
    *(char *)dst++ = ch;
  }
}