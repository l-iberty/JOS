// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <include/elf.h>
#include <include/lib.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/string.h>
#include <kernel/console.h>
#include <kernel/monitor.h>

#define CMDBUF_SIZE 80  // enough for one CGA text line
#define ELFHDR ((struct Elf *)0x10000)

struct Command {
  const char *name;
  const char *desc;
  // return -1 to force monitor to exit
  int (*func)(int argc, char **argv, struct Trapframe *tf);
};

static struct Command commands[] = {
    {"help", "Display this list of commands", mon_help},
    {"kerninfo", "Display information about the kernel", mon_kerninfo},
    {"paginginfo", "Display information about the paging", mon_paginginfo},
    {"reboot", "Reboot the JOS", mon_reboot},
};

/***** Implementations of basic kernel monitor commands *****/

int mon_help(int argc, char **argv, struct Trapframe *tf) {
  int i;

  for (i = 0; i < ARRAY_SIZE(commands); i++) {
    printf("%s - %s\n", commands[i].name, commands[i].desc);
  }
  return 0;
}

int mon_kerninfo(int argc, char **argv, struct Trapframe *tf) {
  extern char _start[], entry[], etext[], edata[], end[];

  struct Proghdr *ph, *eph;
  int i;

  ph = (struct Proghdr *)((uint8_t *)ELFHDR + ELFHDR->e_phoff);
  eph = ph + ELFHDR->e_phnum;

  printf("Special kernel symbols:\n");
  printf("  _start            %x (phys)\n", _start);
  printf("  entry  %x (virt)  %x (phys)\n", entry, entry - KERNBASE);
  printf("  etext  %x (virt)  %x (phys)\n", etext, etext - KERNBASE);
  printf("  edata  %x (virt)  %x (phys)\n", edata, edata - KERNBASE);
  printf("  end    %x (virt)  %x (phys)\n", end, end - KERNBASE);
  printf("Kernel executable memory footprint: %dKB\n", ROUNDUP(end - _start, 1024) / 1024);

  for (i = 1; ph < eph; ph++, i++) {
    printf("segment %d: pa = %x  va = %x  memsz = %x\n", i, ph->p_pa, ph->p_va, ph->p_memsz);
  }

  return 0;
}

int mon_paginginfo(int argc, char **argv, struct Trapframe *tf) {
  extern char entry_pgdir[];
  extern char entry_pgtable[];
  extern int entry_pgdir_size, entry_pgtable_size;

  printf("Information of paging:\n");
  printf("  pgdir   addr:  %x (virt)  %x (phys)\n", entry_pgdir, entry_pgdir - KERNBASE);
  printf("  pgtable addr:  %x (virt)  %x (phys)\n", entry_pgtable, entry_pgtable - KERNBASE);
  printf("  sizeof pgdir:   %d\n", entry_pgdir_size);
  printf("  sizeof pgtable: %d\n", entry_pgtable_size);

  return 0;
}

int mon_reboot(int argc, char **argv, struct Trapframe *tf) {
  printf("Rebooting...");
  outb(0x92, 0x03);

  return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int runcmd(char *buf, struct Trapframe *tf) {
  int argc;
  char *argv[MAXARGS];
  int i;

  // Parse the command buffer into whitespace-separated arguments
  argc = 0;
  argv[argc] = 0;
  while (1) {
    // gobble whitespace
    while (*buf && strchr(WHITESPACE, *buf)) *buf++ = 0;
    if (*buf == 0) break;

    // save and scan past next arg
    if (argc == MAXARGS - 1) {
      printf("Too many arguments (max %d)\n", MAXARGS);
      return 0;
    }
    argv[argc++] = buf;
    while (*buf && !strchr(WHITESPACE, *buf)) buf++;
  }
  argv[argc] = 0;

  // Lookup and invoke the command
  if (argc == 0) return 0;
  for (i = 0; i < ARRAY_SIZE(commands); i++) {
    if (strcmp(argv[0], commands[i].name) == 0) {
      return commands[i].func(argc, argv, tf);
    }
  }
  printf("Unknown command '%s'\n", argv[0]);
  return 0;
}

void monitor(struct Trapframe *tf) {
  char *buf;

  printf("Welcome to the JOS kernel monitor!\n");
  printf("Type 'help' for a list of commands.\n");

  while (1) {
    buf = readline("K> ");
    if (buf != NULL)
      if (runcmd(buf, tf) < 0) break;
  }
}
