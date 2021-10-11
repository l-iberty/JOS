#include <include/stdio.h>
#include <include/string.h>
#include <include/lib.h>

extern uint16_t CGA_PRINT_POS;

void testfunc() {
  printf(">> %d %d %d %d %d %d %d %d %d\n", 0, 1, 9, 10, 92, 100, 101, 987, 1002);
  printf(">> %x %x %x %x %x %x %x %x %x\n", 0, 1, 9, 10, 92, 100, 101, 987, 1002);
  printf(">> %s %s %s %s", "h", "hh", "xhs", "hello   this is kernel...");
  /* Extract cursor location */
  int pos;
  outb(0x3d4, 14);
  pos = inb(0x3d5) << 8;
  outb(0x3d4, 15);
  pos |= inb(0x3d5);
  printf("\nCGA_PRINT_POS: %d\n", CGA_PRINT_POS);
  printf("cursor location: %d\n", pos);
  CGA_PRINT_POS = pos*2;
  printf("test");
}