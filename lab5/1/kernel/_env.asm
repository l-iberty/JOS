global _env_pop_tf

; void _env_pop_tf(struct Trapframe *tf)
_env_pop_tf:
    mov   esp, [esp+4]
    popad
    pop   es
    pop   ds
    add   esp, 8
    iret
