#ifndef JOS_KERN_ENV_ASM_H
#define JOS_KERN_ENV_ASM_H

struct Trapframe;

void __asm_env_pop_tf(struct Trapframe *tf) __attribute__((noreturn));

#endif /* JOS_KERN_ENV_ASM_H */
