global _start

[SECTION .text]
_start:
    mov  ah, 0Ch
    mov  al, 'K'
    mov  ebx, 0B8000h + (80*0+39)*2
    mov  [ebx], ax
    jmp $