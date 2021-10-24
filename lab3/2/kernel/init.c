#include <include/memlayout.h>
#include <include/mmu.h>
#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <kernel/console.h>
#include <kernel/env.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>
#include <kernel/trap.h>

#define ELFHDR ((struct Elf *)0x10000)

void i386_init() {
  extern char edata[], end[];

  // Before doing anything else, complete the ELF loading process.
  // Clear the uninitialized global data (BSS) section of our program.
  // This ensures that all static/global variables start out zero.
  memset(edata, 0, end - edata);

  // Initialize the console.
  // Can't call cprintf until after we do this!
  cons_init();

  kprintf("6828 decimal is %o octal!\n", 6828);

  // Lab 2 memory management initialization functions
  mem_init();

  // Lab 3 user environment initialization functions
  env_init();
  trap_init();

  // Touch all you want.
  ENV_CREATE(user_hello, ENV_TYPE_USER);

  // We only have one user environment for now, so just run it.
  env_run(&envs[0]);
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

  if (panicstr) goto dead;
  panicstr = fmt;

  // Be extra sure that the machine is in as reasonable state
  asm volatile("cli; cld");

  va_start(ap, fmt);
  kprintf("kernel panic at %s:%d: ", file, line);
  vkprintf(fmt, ap);
  kprintf("\n");
  va_end(ap);

dead:
  /* break into the kernel monitor */
  while (1) {
    monitor(NULL);
  }
}

/* like panic, but don't */
void _warn(const char *file, int line, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  kprintf("kernel warning at %s:%d: ", file, line);
  vkprintf(fmt, ap);
  kprintf("\n");
  va_end(ap);
}