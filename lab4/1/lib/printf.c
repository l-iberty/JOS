#include <include/lib.h>
#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/types.h>

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

  sys_puts(buf, strlen(buf));
}