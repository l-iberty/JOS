[SECTION .text]
global _syscall
; int32_t _syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
_syscall:
    mov   eax, [esp+4] ; num
    mov   edx, [esp+8] ; a1
    mov   ecx, [esp+12] ; a2
    mov   ebx, [esp+16] ; a3
    mov   edi, [esp+20] ; a4
    mov   esi, [esp+24] ; a5
    int   0x30
    ret