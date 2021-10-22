#include <include/stdio.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/types.h>
#include <include/mmu.h>
#include <include/memlayout.h>
#include <include/elf.h>
#include <include/lib.h>

#include <kernel/console.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>

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

  mem_init();

  // Drop into the kernel monitor.
  while (1) {
    monitor(NULL);
  }
}

/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void _panic(const char *file, int line, const char *fmt, ...) {
  va_list ap;

  if (panicstr)
    goto dead;
  panicstr = fmt;

  // Be extra sure that the machine is in as reasonable state
  asm volatile("cli; cld");

  va_start(ap, fmt);
  printf("kernel panic at %s:%d: ", file, line);
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);

dead:
  /* break into the kernel monitor */
  while (1){
    monitor(NULL);
  }
}

/* like panic, but don't */
void _warn(const char *file, int line, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("kernel warning at %s:%d: ", file, line);
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}