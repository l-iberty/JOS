global __lib_putc
global inb
global outb

__lib_putc:
    mov  ah, 0Ch
    mov  al, 'K'
    mov  ebx, 0B8000h + (80*0+39)*2
    mov  [ebx], ax
    ret

; uint8_t inb(uint16_t port)
inb:
    mov word dx, [esp+4]  ; port
    xor eax, eax
    in  al, dx
    nop
    nop
    ret

; void outb(uint16_t port, uint8_t byte)
outb:
    mov word dx, [esp+4]  ; port
    mov byte al, [esp+8]  ; byte
    out dx, al
    nop
    nop
    ret