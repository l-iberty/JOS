#include <include/types.h>
#include <include/hd.h>
#include <include/x86.h>
#include <include/elf.h>

/**********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(boot.S and main.c) is the bootloader.  It should
 *    be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in boot.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 **********************************************************************/

#define SECSIZE   512
#define ELFHDR    ((struct Elf *) 0x10000) // scratch space

typedef void(*EntryFn)();

void readsect(void *dst, uint32_t sector);
void readseg(uint32_t pa, uint32_t count, uint32_t offset);

void bootmain() {
  struct Proghdr *ph, *eph;

  // read 1st page off disk
  readseg((uint32_t) ELFHDR, SECSIZE*8, 0);

  // is this a valid ELF?
  if (ELFHDR->e_magic != ELF_MAGIC) {
    goto bad;
  }

  // load each program segment (ignores ph flags)
  ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
  eph = ph + ELFHDR->e_phnum;
  for (; ph < eph; ph++) {
    // p_pa is the load address of this segment (as well
    // as the physical address)
    readseg(ph->p_pa, ph->p_memsz, ph->p_offset);
  }

  // call the entry point from the ELF header
  // note: does not return!
  ((EntryFn) (ELFHDR->e_entry))();

bad:
  asm volatile("hlt");
}

void waitdisk() {
  // wait for disk ready
  while ((x86_inb(REG_STATUS) & (STATUS_BSY | STATUS_DRDY)) != STATUS_DRDY)
    /* do nothing */ ;
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked
//
//  ┌───────────────┐
//  │               │
//  │   sector 0    │  ---->   boot sector
//  │               │
//  ├───────────────┤┈┈┈┈┈┈┈┈┈┈
//  │               │        |
//  │   sector 1    │        |
//  │               │        v
//  ├───────────────┤      kernel
//  │               │
//  │   sector 2    │
//  │               │
//  ├───────────────┤
//  │               │
//  │               │
//
//
//
//
//
void readseg(uint32_t pa, uint32_t count, uint32_t offset) {
  uint32_t end_pa, sector;

  end_pa = pa + count;

  // round down to sector boundary
  pa &= ~(SECSIZE - 1);

  // translate from bytes to sectors, and kernel starts at sector 1
  // offset 是以字节为单位的磁盘偏移量(相对于磁盘开头), 需将其转换为以
  // 扇区(sector)为单位的偏移量, 也就是扇区号. 由于 kernel 从 sector 1 开始,
  // 所以再+1
  sector = (offset / SECSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  while (pa < end_pa) {
    // Since we haven't enabled paging yet and we're using
    // an identity segment mapping (see boot.asm), we can
    // use physical address directly. This won't be the case
    // once JOS enables the MMU.
    readsect((uint8_t*) pa, sector);
    pa += SECSIZE;
    sector++;
  }
}

void readsect(void *dst, uint32_t sector) {
  // wait for disk to be ready
  waitdisk();

  x86_outb(REG_NSECTOR, 1);
  x86_outb(REG_LBA_LOW, sector & 0xFF);
  x86_outb(REG_LBA_MID, (sector >> 8) & 0xFF);
  x86_outb(REG_LBA_HIGH, (sector >> 16) & 0xFF);

  // lba = 1
  // drv = 0 (master), 1 (slave)
  x86_outb(REG_DEVICE, MAKE_DEVICE_REG(1, 0, sector >> 24));
  x86_outb(REG_STATUS, ATA_READ);

  // wait for disk to be ready
  waitdisk();

  // read a sector
  x86_insl(REG_DATA, dst, SECSIZE/4);
}