#include <include/stdio.h>
#include <include/string.h>
#include <include/stdarg.h>

extern void cga_print(const char*);

void printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

void vprintf(const char *fmt, va_list ap) {
  char buf[256];
  memset(buf, 0, sizeof(buf));

  vsprintf(buf, fmt, ap);

  cga_print(buf);
}
