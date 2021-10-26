#include <include/stdio.h>

void umain(int argc, char **argv) {
  // int *p = (int *)0xf0100000;
  asm volatile("int $14");  // page fault
  int *p = (int *)0x0;
  *p = 100;
}