#include <include/stdio.h>
#include <include/string.h>
#include <include/stdarg.h>

#include <kernel/console.h>

void kprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vkprintf(fmt, ap);
  va_end(ap);
}

void vkprintf(const char *fmt, va_list ap) {
  char buf[256];
  memset(buf, 0, sizeof(buf));

  vsprintf(buf, fmt, ap);

  cons_print(buf);
}
