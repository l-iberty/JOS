global inb
global outb
global inw
global outw
global ind
global outd
global insb
global insw
global insd

; uint8_t inb(uint16_t port)
inb:
    mov  word dx, [esp+4]  ; port
    xor  eax, eax
    in   al, dx
    ret

; void outb(uint16_t port, uint8_t byte)
outb:
    mov  word dx, [esp+4]  ; port
    mov  byte al, [esp+8]  ; byte
    out  dx, al
    ret

; uint16_t inw(uint16_t port)
inw:
    mov  word dx, [esp+4]  ; port
    xor  eax, eax
    in   ax, dx
    ret

; void outw(uint16_t port, uint16_t word)
outw:
    mov  word dx, [esp+4]  ; port
    mov  word ax, [esp+8]  ; word
    out  dx, ax
    ret

; uint32_t ind(uint16_t port)
ind:
    mov  word dx, [esp+4]  ; port
    xor  eax, eax
    in   eax, dx
    ret

; void outd(uint16_t port, uint32_t dword)
outd:
    mov  word dx, [esp+4]  ; port
    mov  word eax, [esp+8]  ; dword
    out  dx, eax
    ret

; INS/INSB/INSW/INSD — Input from Port to String
; @see https://www.felixcloutier.com/x86/ins:insb:insw:insd
;
; void insb(uint16_t port, void *addr, int cnt)
insb:
    push  ebp
    mov   ebp, esp
    push  edx
    push  esi
    push  ecx
    mov   edx, [ebp+8]  ; port
    mov   edi, [ebp+12] ; addr
    mov   ecx, [ebp+16] ; cnt
    cld
    rep   insb ; Input byte from I/O port specified in DX into memory location specified in ES:(E)DI
    pop   ecx
    pop   edi
    pop   edx
    pop   ebp
    ret

; void insw(uint16_t port, void *addr, int cnt)
insw:
    push  ebp
    mov   ebp, esp
    push  edx
    push  esi
    push  ecx
    mov   edx, [ebp+8]  ; port
    mov   edi, [ebp+12] ; addr
    mov   ecx, [ebp+16] ; cnt
    cld
    rep   insw ; Input word from I/O port specified in DX into memory location specified in ES:(E)DI
    pop   ecx
    pop   edi
    pop   edx
    pop   ebp
    ret

; void insd(uint16_t port, void *addr, int cnt)
insd:
    push  ebp
    mov   ebp, esp
    push  edx
    push  esi
    push  ecx
    mov   edx, [ebp+8]  ; port
    mov   edi, [ebp+12] ; addr
    mov   ecx, [ebp+16] ; cnt
    cld
    rep   insd ; Input dword from I/O port specified in DX into memory location specified in ES:(E)DI
    pop   ecx
    pop   edi
    pop   edx
    pop   ebp
    ret
