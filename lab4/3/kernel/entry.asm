extern entry_pgdir
extern i386_init

PGSIZE   equ 0x1000
KERNBASE equ 0xF0000000
CR0_PE   equ 0x00000001 ; Protection Enable
CR0_WP   equ 0x00010000 ; Write Protect
CR0_PG   equ 0x80000000 ; Paging
KSTKSIZE equ PGSIZE*8

%define RELOC(x) ((x) - KERNBASE)

global _start
_start equ RELOC(entry)

[SECTION .text]
global entry
entry:
	; We haven't set up virtual memory yet, so we're running from
	; the physical address the boot loader loaded the kernel at: 1MB
	; (plus a few bytes).  However, the C code is linked to run at
	; KERNBASE+1MB.  Hence, we set up a trivial page directory that
	; translates virtual addresses [KERNBASE, KERNBASE+4MB) to
	; physical addresses [0, 4MB).  This 4MB region will be
	; sufficient until we set up our real page table in mem_init
	; in lab 2.

	; Load the physical address of entry_pgdir into cr3.  entry_pgdir
	; is defined in entrypgdir.c.
    mov   eax, entry_pgdir
    sub   eax, KERNBASE    ; we need the physical addr of it
    mov   cr3, eax
    ; Turn on paging
    mov   eax, cr0
    or    eax, CR0_PE|CR0_WP|CR0_PG
    mov   cr0, eax

    ; Now paging is enabled, but we're still running at a low EIP
    ; (why is this okay?).  Jump up above KERNBASE before entering
    ; C code.
    mov   eax, .relocated
    jmp   eax
.relocated:
	; Clear the frame pointer register (EBP)
	; so that once we get into debugging C code,
	; stack backtraces will be terminated properly.
    mov   ebp, 0

    ; set the stack pointer
    mov   esp, bootstacktop

    ; now to C code
    call  i386_init

    jmp $

[SECTION .data]
ALIGN 0x1000
global bootstack
bootstack:
    times KSTKSIZE/4 dd 0
global bootstacktop
bootstacktop: