// test user-level fault handler -- alloc pages to fix faults

#include <include/lib.h>

void handler(struct UTrapframe *utf) {
  int r;
  void *addr = (void *)utf->utf_fault_va;

  printf("fault %p\n", addr);
  if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) < 0) {
    panic("allocating at %08x in page fault handler", addr);
  }
  sprintf((char *)addr, "this string was faulted in at 0x%08x", addr);
}

void umain(int argc, char **argv) {
  set_pgfault_handler(handler);
  printf("%s\n", (char *)0xDeadBeef);
  printf("%s\n", (char *)0xCafebffe);
}