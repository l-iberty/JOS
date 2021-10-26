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

SIZE_PUSHREGS equ 32

%macro trap 1
    pushad
    push   es
    push   ds

    mov   ax, ss
    mov   ds, ax
    mov   es, ax

%endmacro

[SECTION .text]

divide_error:


debug:


non_maskable_interrupt:


breakpoint:


overflow:


bound_range_exceeded:


double_fault:


device_not_avaliable:


double_fault:


coprocessor_segment_overrun:


invalid_tss:


segment_not_present:


stack_fault:


general_fault:


page_fault:


fpu_float_point_error:


alignment_check:


machine_check:


simd_float_point_exception:

