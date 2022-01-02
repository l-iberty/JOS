[SECTION .text]
global syscall
; int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
syscall:
    push  ebp
    mov   ebp, esp
    push  edx
    push  ecx
    push  edi
    push  esi
    push  ebx

    mov   eax, [ebp+8] ; num
    mov   edx, [ebp+12] ; a1
    mov   ecx, [ebp+16] ; a2
    mov   ebx, [ebp+20] ; a3
    mov   edi, [ebp+24] ; a4
    mov   esi, [ebp+28] ; a5
    int   0x30

    pop   ebx
    pop   esi
    pop   edi
    pop   ecx
    pop   edx
    pop   ebp
    ret