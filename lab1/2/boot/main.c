#include "hd.h"

extern void __lib_putc();
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t byte);

void waitdisk();
void readsect(void *dst, uint32_t offset);

void bootmain() {
  __lib_putc();
}

void waitdisk() {
  // wait for disk ready
  while ((inb(REG_STATUS) & (STATUS_BSY | STATUS_DRDY)) != STATUS_DRDY)
    /* do nothing */ ;
}

void readsect(void *dst, uint32_t offset) {
  // wait for disk to be ready
  waitdisk();

  outb(REG_NSECTOR, 1);
  outb(REG_LBA_LOW, offset & 0xFF);
  outb(REG_LBA_MID, (offset >> 8) & 0xFF);
  outb(REG_LBA_HIGH, (offset >> 16) & 0xFF);

  // lba = 1
  // drv = 0 (master), 1 (slave)
  outb(REG_DEVICE, MAKE_DEVICE_REG(1, 0, offset >> 24));

  // wait for disk to be ready

  // read a sector
  insl(REG_DATA, dst, SECSIZE/4);
}