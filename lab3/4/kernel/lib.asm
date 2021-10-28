global inb
global outb
global inw
global outw
global ind
global outd
global insb
global insw
global insd
global rcr0
global rcr2
global rcr3
global lcr0
global lcr2
global lcr3
global invlpg
global lgdt
global lldt
global lidt
global ltr
global read_eflags
global write_eflags

[SECTION .text]

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
    mov  word dx, [esp+4]   ; port
    mov  dword eax, [esp+8] ; dword
    out  dx, eax
    ret

; INS/INSB/INSW/INSD — Input from Port to String
; @see https://www.felixcloutier.com/x86/ins:insb:insw:insd
;
; void insb(uint16_t port, void *addr, int cnt)
insb:
    mov   edx, [ebp+4]  ; port
    mov   edi, [ebp+8]  ; addr
    mov   ecx, [ebp+12] ; cnt
    cld
    rep   insb ; Input byte from I/O port specified in DX into memory location specified in ES:(E)DI
    ret

; void insw(uint16_t port, void *addr, int cnt)
insw:
    mov   edx, [ebp+4]  ; port
    mov   edi, [ebp+8] ; addr
    mov   ecx, [ebp+12] ; cnt
    cld
    rep   insw ; Input word from I/O port specified in DX into memory location specified in ES:(E)DI
    ret

; void insd(uint16_t port, void *addr, int cnt)
insd:
    mov   edx, [ebp+4]  ; port
    mov   edi, [ebp+8] ; addr
    mov   ecx, [ebp+12] ; cnt
    cld
    rep   insd ; Input dword from I/O port specified in DX into memory location specified in ES:(E)DI
    ret

; uint32_t rcr0();
rcr0:
    mov   eax, cr0
    ret

; uint32_t rcr2();
rcr2:
    mov   eax, cr2
    ret

; uint32_t rcr3();
rcr3:
    mov   eax, cr3
    ret

; void lcr0(uint32_t x);
lcr0:
    mov   eax, [esp+4]
    mov   cr0, eax
    ret

; void lcr2(uint32_t x);
lcr2:
    mov   eax, [esp+4]
    mov   cr2, eax
    ret

; void lcr3(uint32_t x);
lcr3:
    mov   eax, [esp+4]
    mov   cr3, eax
    ret

; void invlpg(void *addr)
invlpg:
    mov     eax, [esp+4]
    invlpg  [eax]
    ret

; void lgdt(void *addr)
lgdt:
    mov   eax, [esp+4]
    lgdt  [eax]
    ret

; void lldt(void *addr)
lldt:
    mov   eax, [esp+4]
    lldt  ax
    ret

; void lidt(void *addr)
lidt:
    mov   eax, [esp+4]
    lidt  [eax]
    ret

; void ltr(uint16_t selector_tss);
ltr:
    mov   eax, [esp+4]
    ltr   ax
    ret

; uint32_t read_eflags();
read_eflags:
    pushf
    pop   eax
    ret

; void write_eflags(uint32_t eflags);
write_eflags:
    push   dword [esp+4]
    popf
    ret