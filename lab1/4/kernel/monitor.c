// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <include/stdio.h>
#include <include/string.h>
#include <include/memlayout.h>
#include <include/x86.h>

#include <kernel/console.h>
#include <kernel/monitor.h>

#define CMDBUF_SIZE	80	// enough for one CGA text line


struct Command {
  const char *name;
  const char *desc;
  // return -1 to force monitor to exit
  int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
  { "help", "Display this list of commands", mon_help },
  { "kerninfo", "Display information about the kernel", mon_kerninfo },
};

/***** Implementations of basic kernel monitor commands *****/

int mon_help(int argc, char **argv, struct Trapframe *tf) {
  int i;

  for (i = 0; i < ARRAY_SIZE(commands); i++)
    printf("%s - %s\n", commands[i].name, commands[i].desc);
  return 0;
}

int mon_kerninfo(int argc, char **argv, struct Trapframe *tf) {
  extern char _start[], etext[], edata[], end[];

  printf("Special kernel symbols:\n");
  printf("  _start  %x (virt)  %x (phys)\n", _start, _start - KERNBASE);
  printf("  etext  %x (virt)  %x (phys)\n", etext, etext - KERNBASE);
  printf("  edata  %x (virt)  %x (phys)\n", edata, edata - KERNBASE);
  printf("  end    %x (virt)  %x (phys)\n", end, end - KERNBASE);
  printf("Kernel executable memory footprint: %dKB\n", ROUNDUP(end - _start, 1024) / 1024);
  return 0;
}