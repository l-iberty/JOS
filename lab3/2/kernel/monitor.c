// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <include/lib.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/string.h>
#include <kernel/console.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>

#define CMDBUF_SIZE 80  // enough for one CGA text line
#define ELFHDR ((struct Elf *)0x10000)

struct Command {
  const char *name;
  const char *desc;
  // return -1 to force monitor to exit
  int (*func)(int argc, char **argv, struct Trapframe *tf);
};

static struct Command commands[] = {{"help", "Display this list of commands", mon_help},
                                    {"kerninfo", "Display information about the kernel", mon_kerninfo},
                                    {"reboot", "Reboot the JOS", mon_reboot},
                                    {"showmappings",
                                     "display the physical page mappings and corresponding permission bits that apply "
                                     "to the pages at virtual addresses",
                                     mon_showmappings}};

/***** Implementations of basic kernel monitor commands *****/

int mon_help(int argc, char **argv, struct Trapframe *tf) {
  int i;

  for (i = 0; i < ARRAY_SIZE(commands); i++) {
    kprintf("%s - %s\n", commands[i].name, commands[i].desc);
  }
  return 0;
}

int mon_kerninfo(int argc, char **argv, struct Trapframe *tf) {
  extern char _start[], entry[], etext[], edata[], end[];

  kprintf("Special kernel symbols:\n");
  kprintf("  _start                    %x (phys)\n", _start);
  kprintf("  entry  %x (virt)  %x (phys)\n", entry, entry - KERNBASE);
  kprintf("  etext  %x (virt)  %x (phys)\n", etext, etext - KERNBASE);
  kprintf("  edata  %x (virt)  %x (phys)\n", edata, edata - KERNBASE);
  kprintf("  end    %x (virt)  %x (phys)\n", end, end - KERNBASE);
  kprintf("Kernel executable memory footprint: %dKB\n", ROUNDUP(end - entry, 1024) / 1024);

  return 0;
}

int mon_showmappings(int argc, char **argv, struct Trapframe *tf) {
  extern pde_t *kern_pgdir;
  pte_t *pgtable, pte;
  uintptr_t va, va_start, va_end;
  physaddr_t pa;
  uint32_t perm;
  char perm_buf[100], *p;

  if (argc != 3) {
    kprintf("Usage: showmappings va1 va2 (hex, page-aligned)\n");
    return 1;
  }

  va_start = strtol(argv[1], NULL, 16);
  va_end = strtol(argv[2], NULL, 16);

  for (va = va_start; va <= va_end; va += PGSIZE) {
    pgtable = (pte_t *)PTE_ADDR(kern_pgdir[PDX(va)]);
    if (pgtable == NULL) {
      pa = 0, perm = 0;
    } else {
      pgtable = (pte_t *)KADDR((physaddr_t)pgtable);
      pte = pgtable[PTX(va)];
      if (pte == 0) {
        pa = 0, perm = 0;
      } else {
        pa = PTE_ADDR(pte);
        perm = PTE_PERM(pte);
      }
    }
    if (pa == 0 && perm == 0) {
      kprintf("%x (virt) -> ??? (phys) ??? (perm)\n", va);
    } else {
      p = perm_buf;
      if (perm & PTE_P) {
        strcpy(p, "Present");
        p += strlen(p);
      }
      if (perm & PTE_W) {
        strcpy(p, " Writable");
        p += strlen(p);
      }
      if (perm & PTE_U) {
        strcpy(p, " User");
        p += strlen(p);
      }
      if (perm & PTE_PWT) {
        strcpy(p, " Write-Through");
        p += strlen(p);
      }
      if (perm & PTE_PCD) {
        strcpy(p, " Cache-Disable");
        p += strlen(p);
      }
      if (perm & PTE_A) {
        strcpy(p, " Accessed");
        p += strlen(p);
      }
      if (perm & PTE_D) {
        strcpy(p, " Dirty");
        p += strlen(p);
      }
      if (perm & PTE_PS) {
        strcpy(p, " Page-Size");
        p += strlen(p);
      }
      if (perm & PTE_G) {
        strcpy(p, " Global");
        p += strlen(p);
      }
      kprintf("%x (virt) -> %x (phys) %s (perm)\n", va, pa, perm_buf);
    }
  }
  return 0;
}

int mon_reboot(int argc, char **argv, struct Trapframe *tf) {
  kprintf("Rebooting...");
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
      kprintf("Too many arguments (max %d)\n", MAXARGS);
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
  kprintf("Unknown command '%s'\n", argv[0]);
  return 0;
}

void monitor(struct Trapframe *tf) {
  char *buf;

  kprintf("Welcome to the JOS kernel monitor!\n");
  kprintf("Type 'help' for a list of commands.\n");

  while (1) {
    buf = readline("K> ");
    if (buf != NULL)
      if (runcmd(buf, tf) < 0) break;
  }
}
