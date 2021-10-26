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

extern trap ; kernel/trap.c => void trap(struct Trapframe *tf)

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
