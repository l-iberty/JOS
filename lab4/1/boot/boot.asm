%include "boot.inc"

; Start the CPU: switch to 32-bit protected mode, jump into C.
; The BIOS loads this code from the first sector of the hard disk into
; memory at physical address 0x7c00 and starts executing in real mode
; with cs=0 ip=7c00.

extern bootmain

[BITS 16]                         ; Assemble for 16-bit mode
global start
start:
  cli                             ; Disable interrupts
  cld                             ; String operations increment

  ; Set up the important data segment registers (DS, ES, SS).
  xor    ax, ax
  mov    ds, ax
  mov    es, ax
  mov    ss, ax

  ; Enable A20:
  ;   For backwards compatibility with the earliest PCs, physical
  ;   address line 20 is tied low, so that addresses higher than
  ;   1MB wrap around to zero by default.  This code undoes this.
seta20.1:
  in     al, 64h                   ; Wait for not busy
  test   al, 2
  jnz    seta20.1

  mov    al, 0D1h
  out    64h, al                   ; 0xD1 -> port 0x64

seta20.2:
  in     al, 64h                   ; Wait for not busy
  test   al, 2
  jnz    seta20.2

  mov    al, 0DFh
  out    60h, al                   ; 0xDF -> port 0x60

  ; Switch from real to protected mode, using a bootstrap GDT
  ; and segment translation that makes virtual addresses
  ; identical to their physical addresses, so that the
  ; effective memory map does not change during the switch.
  lgdt   [GDT_DESC]
  mov    eax, cr0
  or     eax, 1                    ; protected mode enable flag
  mov    cr0, eax

  ; Switches processor into 32-bit mode.
  jmp    dword SELECTOR_FLATC:LABEL_PROT_CSEG

[BITS 32]                         ; Assemble for 32-bit mode
LABEL_PROT_CSEG:
  ; Set up the protected-mode data segment registers
  mov    ax, SELECTOR_FLATRW
  mov    ds, ax
  mov    es, ax
  mov    ss, ax
  mov    gs, ax
  mov    fs, ax

  ; Set up the stack pointer and call into C.
  mov    esp, start
  call   bootmain

  ; If bootmain returns (it shouldn't), loop.
  jmp    $


; GDT
;                                   Base    Limit     Attr
ALIGN 4
LABEL_GDT:            Descriptor    0,      0,        0
LABEL_DESC_FLAT_C:    Descriptor    0,      0FFFFFh,  DA_C32 | DA_G_4K
LABEL_DESC_FLAT_RW:   Descriptor    0,      0FFFFFh,  DA_D32 | DA_G_4K

GDT_SIZE equ $ - LABEL_GDT
GDT_DESC: dw  GDT_SIZE - 1  ; limit
          dd  LABEL_GDT     ; base

; selectors
SELECTOR_FLATC  equ  LABEL_DESC_FLAT_C  - LABEL_GDT
SELECTOR_FLATRW equ  LABEL_DESC_FLAT_RW - LABEL_GDT
