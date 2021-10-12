#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mmu.h>
#include <include/memlayout.h>

#include <kernel/console.h>

void i386_init() {
  extern char _start[], etext[], edata[], end[];
  extern char entry_pgdir[];
  extern char entry_pgtable[];
  extern int entry_pgdir_size, entry_pgtable_size;

  cons_init();

  printf("6828 is %o octal!\n", 6828);

  printf("\nSpecial kernel symbols:\n");
  printf("  _start  %x (virt)  %x (phys)\n", _start, _start - KERNBASE);
  printf("  etext   %x (virt)  %x (phys)\n", etext, etext - KERNBASE);
  printf("  edata   %x (virt)  %x (phys)\n", edata, edata - KERNBASE);
  printf("  end     %x (virt)  %x (phys)\n", end, end - KERNBASE);
  printf("Kernel executable memory footprint: %dKB\n", ROUNDUP(end - _start, 1024) / 1024);

  printf("\nInfomation of paging:\n");
  printf("  pgdir   addr:  %x (virt)  %x (phys)\n", entry_pgdir, entry_pgdir - KERNBASE);
  printf("  pgtable addr:  %x (virt)  %x (phys)\n", entry_pgtable, entry_pgtable - KERNBASE);
  printf("  sizeof pgdir:   %d\n", entry_pgdir_size);
  printf("  sizeof pgtable: %d\n", entry_pgtable_size);

  for (;;)
    /* do nothing */ ;
}