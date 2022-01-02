#include <include/memlayout.h>
#include <include/mmu.h>
#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/x86.h>
#include <kernel/console.h>
#include <kernel/cpu.h>
#include <kernel/env.h>
#include <kernel/monitor.h>
#include <kernel/picirq.h>
#include <kernel/pmap.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>
#include <kernel/trap.h>

#define ELFHDR ((struct Elf *)0x10000)

static void boot_aps(void);

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

  // Lab 2 memory management initialization functions
  mem_init();

  // Lab 3 user environment initialization functions
  env_init();
  trap_init();

  // Lab 4 multiprocessor initialization functions
  mp_init();
  lapic_init();

  // Lab 4 multitasking initialization functions
  pic_init();

  // Acquire the big kernel lock before waking up APs
  // Your code here:
  lock_kernel();

  // Starting non-boot CPUs
  boot_aps();

  // Touch all you want.
  // ENV_CREATE(user_hello, ENV_TYPE_USER);
  // ENV_CREATE(user_divzero, ENV_TYPE_USER);
  // ENV_CREATE(user_badsegment, ENV_TYPE_USER);
  // ENV_CREATE(user_softint, ENV_TYPE_USER);
  // ENV_CREATE(user_invlopcode, ENV_TYPE_USER);
  // ENV_CREATE(user_faultread, ENV_TYPE_USER);
  // ENV_CREATE(user_faultreadkernel, ENV_TYPE_USER);
  // ENV_CREATE(user_faultwrite, ENV_TYPE_USER);
  // ENV_CREATE(user_faultwritekernel, ENV_TYPE_USER);
  // ENV_CREATE(user_breakpoint, ENV_TYPE_USER);

  // ENV_CREATE(user_yield, ENV_TYPE_USER);
  // ENV_CREATE(user_yield, ENV_TYPE_USER);
  // ENV_CREATE(user_yield, ENV_TYPE_USER);
  // ENV_CREATE(user_yield, ENV_TYPE_USER);
  // ENV_CREATE(user_yield, ENV_TYPE_USER);

  // ENV_CREATE(user_dumbfork, ENV_TYPE_USER);

  // ENV_CREATE(user_faultalloc, ENV_TYPE_USER);
  // ENV_CREATE(user_faultallocbad, ENV_TYPE_USER);
  // ENV_CREATE(user_faultregs, ENV_TYPE_USER);
  // ENV_CREATE(user_faultbadhandler, ENV_TYPE_USER);
  // ENV_CREATE(user_faultevilhandler, ENV_TYPE_USER);
  // ENV_CREATE(user_faultnostack, ENV_TYPE_USER);

  // ENV_CREATE(user_forktree, ENV_TYPE_USER);

  // ENV_CREATE(user_spin, ENV_TYPE_USER);

  // ENV_CREATE(user_stresssched, ENV_TYPE_USER);

  // ENV_CREATE(user_pingpong, ENV_TYPE_USER);
  ENV_CREATE(user_primes, ENV_TYPE_USER);
  // ENV_CREATE(user_sendpage, ENV_TYPE_USER);

  // Schedule and run the first user environment!
  sched_yield();
}

// While boot_aps is booting a given CPU, it communicates the per-core
// stack pointer that should be loaded by mpentry.S to that CPU in
// this variable.
void *mpentry_kstack;

// Start the non-boot (AP) processors.
static void boot_aps(void) {
  extern unsigned char mpentry_start[], mpentry_end[];
  void *code;
  struct CpuInfo *c;

  // Write entry code to unused memory at MPENTRY_PADDR
  code = KADDR(MPENTRY_PADDR);
  memmove(code, mpentry_start, mpentry_end - mpentry_start);

  // Boot each AP one at a time
  for (c = cpus; c < cpus + ncpu; c++) {
    if (c == cpus + cpunum())  // We've started already.
      continue;

    // Tell mpentry.S what stack to use
    mpentry_kstack = percpu_kstacks[c - cpus] + KSTKSIZE;
    // Start the CPU at mpentry_start
    lapic_startap(c->cpu_id, PADDR(code));
    // Wait for the CPU to finish some basic setup in mp_main()
    while (c->cpu_status != CPU_STARTED)
      ;
  }
}

// Setup code for APs
void mp_main(void) {
  // We are in high EIP now, safe to switch to kern_pgdir
  lcr3(PADDR(kern_pgdir));
  printf("SMP: CPU %d starting\n", cpunum());

  lapic_init();
  env_init_percpu();
  trap_init_percpu();
  xchg(&thiscpu->cpu_status, CPU_STARTED);  // tell boot_aps() we're up

  // Now that we have finished some basic setup, call sched_yield()
  // to start running processes on this CPU.  But make sure that
  // only one CPU can enter the scheduler at a time!
  //
  // Your code here:
  lock_kernel();

  sched_yield();
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
  printf("kernel panic at %s:%d: ", file, line);
  vprintf(fmt, ap);
  printf("\n");
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
  printf("kernel warning at %s:%d: ", file, line);
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}