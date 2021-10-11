global _start

extern cga_print
extern testfunc

[SECTION .data]
    KERNEL_MSG  db 'Hello, JOS Kernel', 0Ah, 0

[SECTION .text]
_start:
    mov   eax, KERNEL_MSG
    push  eax
    call  cga_print
    add   esp, 4
    call  testfunc
    jmp $