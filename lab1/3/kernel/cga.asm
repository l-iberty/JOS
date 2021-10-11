global cga_putc
global cga_print

%include "include/cga.inc"

[SECTION .data]
    CGA_PRINT_POS   dd  0

[SECTION .text]

; void putc(char ch)
cga_putc:
    mov   edi, [CGA_PRINT_POS]
    mov   ah, BLACK | LIGHT_RED  ; 黑底红字
    mov   al, [esp+4]   ; ch
    cmp   al, 0Ah       ; '\n' ?
    jnz   .1
    call  __cga_newline
    mov   [CGA_PRINT_POS], edi
    jmp   .2
.1:
    mov   [edi+CGA_BASE], ax
    add   dword [CGA_PRINT_POS], 2
.2:
    ret

; 默认 edi 存放显示位置, __newline 改变 edi 的内容, 使其指向下一行行首:
; edi = (edi / 160) * 160 + 160
__cga_newline:
    push  eax
    push  edx
    mov   eax, edi
    mov   ebx, 160
    div   bl ; 16 bits 被除数 AX, 8 bits 除数 BL
             ; AH = 余数, AL = 商
    and   eax, 0FFh ; 取 AL
    mul   ebx       ; (EDX,EAX) <- (EAX)*(SRC)
    mov   edi, eax  ; 当前行首
    add   edi, 160  ; 下一行行首
    pop   ebx
    pop   eax
    ret

; void print(char *s)
cga_print:
    mov   esi, [esp+4]  ; addr of s
.1:
    mov   al, [esi]
    cmp   al, 0
    jz    .3
    push  eax
    call  cga_putc
    add   esp, 4
.2:
    inc   esi
    jmp   .1
.3:
    ret