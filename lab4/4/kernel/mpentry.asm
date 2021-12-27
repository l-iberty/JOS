extern entry_pgdir
extern mp_main
extern mpentry_kstack

KERNBASE equ 0xF0000000
CR0_PE   equ 0x00000001 ; Protection Enable
CR0_WP   equ 0x00010000 ; Write Protect
CR0_PG   equ 0x80000000 ; Paging

MPENTRY_PADDR equ 0x7000

; 描述符
; usage: Descriptor Base, Limit, Attr
;	 %1 -> Base:	dd, 32　bits段基址(线性基地址)
;	 %2 -> Limit:	dd, 20　bits段界限(低 20　bits 可用)
;	 %3 -> Attr:	dw, 12　bits属性(高字节的低 4　bits 恒为0)
%macro Descriptor 3
	dw	%2 & 0FFFFh                         ; 段界限1
	dw	%1 & 0FFFFh                         ; 段基址1
	db	(%1 >> 16) & 0FFh                   ; 段基址2
	dw	(%3 & 0F0FFh) | ((%2 >> 8) & 0F00h) ; 属性 + 段界限2
	db	(%1 >> 24) & 0FFh                   ; 段基址3
%endmacro ; 8 Bytes

; 描述符属性
;	DA_: Descriptor Attribute
DA_C32		equ	0_1_0_0_0000_1_00_1_1000b  ; 32位代码段属性 ( TYPE=1000b -> 只执行 )
DA_D32		equ	0_1_0_0_0000_1_00_1_0010b  ; 32位数据段属性 ( TYPE=0010b -> 可读写, 向高地址扩展 )
DA_G_4K		equ	1_0_0_0_0000_0_00_0_0000b  ; 4K 粒度

%define RELOC(x) ((x) - KERNBASE)
%define MPBOOTPHYS(s) ((s) - mpentry_start + MPENTRY_PADDR)

[BITS 16]
global mpentry_start
mpentry_start:
  cli

  xor    ax, ax
  mov    ds, ax
  mov    es, ax
  mov    ss, ax

  lgdt   [MPBOOTPHYS(GDT_DESC)]
  mov    eax, cr0
  or     eax, CR0_PE
  mov    cr0, eax

  jmp    dword SELECTOR_FLATC:MPBOOTPHYS(LABEL_PROT_CSEG)

[BITS 32]
LABEL_PROT_CSEG:
  mov    ax, SELECTOR_FLATRW
  mov    ds, ax
  mov    es, ax
  mov    ss, ax
  mov    gs, ax
  mov    fs, ax

  ; Set up initial page table. We cannot use kern_pgdir yet because
  ; we are still running at a low EIP.
  mov    eax, RELOC(entry_pgdir)
  mov    cr3, eax
  ; Turn on paging.
  mov    eax, cr0
  or     eax, CR0_PE|CR0_WP|CR0_PG
  mov    cr0, eax

  ; Switch to the per-cpu stack allocated in boot_aps()
  mov    esp, [mpentry_kstack]
  mov    ebp, 0

  ; Call mp_main().  (why the indirect call?)
  mov    eax, mp_main
  call   eax

  ; If mp_main returns (it shouldn't), loop.
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

global mpentry_end
mpentry_end:
