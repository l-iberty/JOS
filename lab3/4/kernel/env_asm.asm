global __asm_env_pop_tf

; void __asm_env_pop_tf(struct Trapframe *tf)
__asm_env_pop_tf:
    mov   esp, [esp+4]
    popad
    pop   es
    pop   ds
    add   esp, 8
    iret
