#ifndef JOS_INC_LIB_H
#define JOS_INC_LIB_H

#include <include/types.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t byte);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t word);
uint32_t ind(uint16_t port);
void outd(uint16_t port, uint32_t dword);
void insb(uint16_t port, void *addr, int cnt);
void insw(uint16_t port, void *addr, int cnt);
void insd(uint16_t port, void *addr, int cnt);

uint32_t rcr0();
uint32_t rcr2();
uint32_t rcr3();
void lcr0(uint32_t x);
void lcr2(uint32_t x);
void lcr3(uint32_t x);

void invlpg(void *addr);

void lgdt(void *addr);
void lldt(void *addr);
void lidt(void *addr);
void ltr(uint16_t selector_tss);

uint32_t read_eflags();
void write_eflags(uint32_t eflags);

#endif /* JOS_INC_LIB_H */