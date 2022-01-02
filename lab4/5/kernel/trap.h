/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H

#include <include/mmu.h>
#include <include/trap.h>

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];
extern struct Pseudodesc idt_pd;

void trap_init(void);
void trap_init_percpu(void);
void print_regs(struct PushRegs *regs);
void print_trapframe(struct Trapframe *tf);
void page_fault_handler(struct Trapframe *);
void backtrace(struct Trapframe *);

void divide_error();
void debug();
void non_maskable_interrupt();
void breakpoint();
void overflow();
void bound_range_exceeded();
void invalid_opcode();
void device_not_avaliable();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_fault();
void general_fault();
void page_fault();
void fpu_float_point_error();
void alignment_check();
void machine_check();
void simd_float_point_exception();
void syscall_handler();

// 使用以下脚本生成代码:
//
// #!/bin/bash
//
// for i in $(seq 1 16); do
//   irq_num=$((i-1))
//   echo "void irq_${irq_num}();"
// done

void irq_0();
void irq_1();
void irq_2();
void irq_3();
void irq_4();
void irq_5();
void irq_6();
void irq_7();
void irq_8();
void irq_9();
void irq_10();
void irq_11();
void irq_12();
void irq_13();
void irq_14();
void irq_15();

#endif /* JOS_KERN_TRAP_H */
