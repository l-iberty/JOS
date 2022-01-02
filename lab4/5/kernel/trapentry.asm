global divide_error
global debug
global non_maskable_interrupt
global breakpoint
global overflow
global bound_range_exceeded
global invalid_opcode
global device_not_avaliable
global double_fault
global coprocessor_segment_overrun
global invalid_tss
global segment_not_present
global stack_fault
global general_fault
global page_fault
global fpu_float_point_error
global alignment_check
global machine_check
global simd_float_point_exception
global syscall_handler

; 使用以下脚本生成代码:
;
; #!/bin/bash
;
; for i in $(seq 1 16); do
;   irq_num=$((i-1))
;   echo "global irq_${irq_num}"
; done

global irq_0
global irq_1
global irq_2
global irq_3
global irq_4
global irq_5
global irq_6
global irq_7
global irq_8
global irq_9
global irq_10
global irq_11
global irq_12
global irq_13
global irq_14
global irq_15

extern trap ; kernel/trap.c => void trap(struct Trapframe *tf)

IRQ_OFFSET equ 0x20

%macro trap_handler 1
    push   %1
    push   ds
    push   es
    pushad

    mov   ax, ss ; ss == GD_KD
    mov   ds, ax
    mov   es, ax

    push  esp
    call  trap
%endmacro

[SECTION .text]

syscall_handler:
    push            0
    trap_handler    0x30

divide_error:
    push            0  ; no errcode,  the same below
    trap_handler    0

debug:
    push            0
    trap_handler    1

non_maskable_interrupt:
    push            0
    trap_handler    2

breakpoint:
    push            0
    trap_handler    3

overflow:
    push            0
    trap_handler    4

bound_range_exceeded:
    push            0
    trap_handler    5

invalid_opcode:
    push            0
    trap_handler    6

device_not_avaliable:
    push            0
    trap_handler    7

double_fault:
    trap_handler    8

coprocessor_segment_overrun:
    push            0
    trap_handler    9

invalid_tss:
    trap_handler    10

segment_not_present:
    trap_handler    11

stack_fault:
    trap_handler    12

general_fault:
    trap_handler    13

page_fault:
    trap_handler    14

fpu_float_point_error:
    push            0
    trap_handler    16

alignment_check:
    trap_handler    17

machine_check:
    push            0
    trap_handler    18

simd_float_point_exception:
    push            0
    trap_handler    19

; 使用以下脚本生成代码:
;
; #!/bin/bash
;
; for i in $(seq 1 16); do
;   irq_num=$((i-1))
;   echo "irq_${irq_num}:"
;   echo "    push            0"
;   echo "    trap_handler    IRQ_OFFSET+${irq_num}"
;   echo ""
; done

irq_0:
    push            0
    trap_handler    IRQ_OFFSET+0

irq_1:
    push            0
    trap_handler    IRQ_OFFSET+1

irq_2:
    push            0
    trap_handler    IRQ_OFFSET+2

irq_3:
    push            0
    trap_handler    IRQ_OFFSET+3

irq_4:
    push            0
    trap_handler    IRQ_OFFSET+4

irq_5:
    push            0
    trap_handler    IRQ_OFFSET+5

irq_6:
    push            0
    trap_handler    IRQ_OFFSET+6

irq_7:
    push            0
    trap_handler    IRQ_OFFSET+7

irq_8:
    push            0
    trap_handler    IRQ_OFFSET+8

irq_9:
    push            0
    trap_handler    IRQ_OFFSET+9

irq_10:
    push            0
    trap_handler    IRQ_OFFSET+10

irq_11:
    push            0
    trap_handler    IRQ_OFFSET+11

irq_12:
    push            0
    trap_handler    IRQ_OFFSET+12

irq_13:
    push            0
    trap_handler    IRQ_OFFSET+13

irq_14:
    push            0
    trap_handler    IRQ_OFFSET+14

irq_15:
    push            0
    trap_handler    IRQ_OFFSET+15
