#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mmu.h>
#include <include/memlayout.h>
#include <include/elf.h>
#include <include/lib.h>

#include <kernel/console.h>
#include <kernel/monitor.h>

#define ELFHDR    ((struct Elf *) 0x10000)

void i386_init() {
  extern char edata[], end[];

  // Before doing anything else, complete the ELF loading process.
  // Clear the uninitialized global data (BSS) section of our program.
  // This ensures that all static/global variables start out zero.
  memset(edata, 0, end - edata);

  // Initialize the console.
  // Can't call cprintf until after we do this!
  cons_init();

  printf("6828 decimal is %o octal!\n", 6828);

  // Drop into the kernel monitor.
  while (1) {
    monitor(NULL);
  }
}