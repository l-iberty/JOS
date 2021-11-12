#ifndef JOS_KERN_PICIRQ_H
#define JOS_KERN_PICIRQ_H

#define MAX_IRQS 16  // Number of IRQs

// I/O Addresses of the two 8259A programmable interrupt controllers
#define IO_PIC1 0x20  // Master (IRQs 0-7)
#define IO_PIC2 0xA0  // Slave (IRQs 8-15)

#define IRQ_SLAVE 2  // IRQ at which slave connects to master

#include <include/types.h>

extern uint16_t irq_mask_8259A;
void pic_init(void);
void irq_setmask_8259A(uint16_t mask);

#endif /* JOS_KERN_PICIRQ_H */