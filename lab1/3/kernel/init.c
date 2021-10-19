#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mmu.h>
#include <include/memlayout.h>
#include <include/elf.h>
#include <include/lib.h>

#include <kernel/console.h>

#define ELFHDR    ((struct Elf *) 0x10000) // scratch space

void i386_init() {
  extern char _start[], entry[], etext[], edata[], end[];
  extern char entry_pgdir[];
  extern char entry_pgtable[];
  extern int entry_pgdir_size, entry_pgtable_size;

  struct Proghdr *ph, *eph;
  int i;

  ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
  eph = ph + ELFHDR->e_phnum;

  cons_init();

  printf("\nSpecial kernel symbols:\n");
  printf("  _start:                    %x (phys)\n", _start);
  printf("  entry   %x (virt)  %x (phys)\n", entry, entry - KERNBASE);
  printf("  etext   %x (virt)  %x (phys)\n", etext, etext - KERNBASE);
  printf("  edata   %x (virt)  %x (phys)\n", edata, edata - KERNBASE);
  printf("  end     %x (virt)  %x (phys)\n", end, end - KERNBASE);
  for (i = 1; ph < eph; ph++, i++) {
    printf("segment %d: pa = %x  va = %x  memsz = %x\n", i, ph->p_pa, ph->p_va, ph->p_memsz);
  }
  printf("Kernel executable memory footprint: %dKB\n", ROUNDUP(end - entry, 1024) / 1024);

  printf("\nInfomation of paging:\n");
  printf("  pgdir   addr:  %x (virt)  %x (phys)\n", entry_pgdir, entry_pgdir - KERNBASE);
  printf("  pgtable addr:  %x (virt)  %x (phys)\n", entry_pgtable, entry_pgtable - KERNBASE);
  printf("  sizeof pgdir:   %d\n", entry_pgdir_size);
  printf("  sizeof pgtable: %d\n", entry_pgtable_size);

  for (;;)
    /* do nothing */ ;
}
