#include <include/lib.h>
#include <include/string.h>

void umain(int argc, char **argv) {
  int r = sys_getenvid();
  sys_puts("hello\n", strlen("hello\n"));
  for (;;)
    ;
}