; Page fault upcall entrypoint.

; This is where we ask the kernel to redirect us to whenever we cause
; a page fault in user space (see the call to sys_set_pgfault_handler
; in pgfault.c).
;
; When a page fault actually occurs, the kernel switches our ESP to
; point to the user exception stack if we're not already on the user
; exception stack, and then it pushes a UTrapframe onto our user
; exception stack:
;
;   /* +48 */ trap-time esp
;   /* +44 */ trap-time eflags
;   /* +40 */ trap-time eip
;   /* +36 */ utf_regs.reg_eax
;             ...
;   /* +12 */ utf_regs.reg_esi
;   /* +8  */ utf_regs.reg_edi
;   /* +4  */ utf_err (error code)
;   /* +0  */ utf_fault_va            <-- %esp
;
; If this is a recursive fault, the kernel will reserve for us a
; blank word above the trap-time esp for scratch work when we unwind
; the recursive call.
;
; We then have call up to the appropriate page fault handler in C
; code, pointed to by the global variable '_pgfault_handler'

extern _pgfault_handler

[SECTION .text]
global _pgfault_upcall
_pgfault_upcall:
    ; call the C page fault handler
    push  esp                     ; function argument: pointer to UTrapframe
    mov   eax, [_pgfault_handler]
    call  eax
    add   esp, 4                  ; pop function argument

    ; Now the C page fault handler has returned and you must return
    ; to the trap time state.
    ; Push trap-time %eip onto the trap-time stack.
    ;
    ; Explanation:
    ;   We must prepare the trap-time stack for our eventual return to
    ;   re-execute the instruction that faulted.
    ;   Unfortunately, we can't return directly from the exception stack:
    ;   We can't call 'jmp', since that requires that we load the address
    ;   into a register, and all registers must have their trap-time
    ;   values after the return.
    ;   We can't call 'ret' from the exception stack either, since if we
    ;   did, %esp would have the wrong value.
    ;   So instead, we push the trap-time %eip onto the *trap-time* stack!
    ;   Below we'll switch to that stack and call 'ret', which will
    ;   restore %eip to its pre-fault value.
    ;
    ;   In the case of a recursive fault on the exception stack,
    ;   note that the word we're pushing now will fit in the
    ;   blank word that the kernel reserved for us.
    ;
    ; Throughout the remaining code, think carefully about what
    ; registers are available for intermediate calculations.  You
    ; may find that you have to rearrange your code in non-obvious
    ; ways as registers become unavailable as scratch space.
    ;
    ; LAB 4: Your code here.
    mov  eax, [esp + 40]    ; %eax <- trap-time %eip
    mov  ebx, [esp + 48]    ; %ebx <- trap-time %esp
    sub  ebx, 4
    mov  [ebx], eax         ; "push" trap-time %eip onto the trap-time stack
    mov  [esp + 48], ebx

    ; Restore the trap-time registers.  After you do this, you
    ; can no longer modify any general-purpose registers.
    ; LAB 4: Your code here.
    add  esp, 8             ; skip utf_fault_va and utf_err
    popa                    ; restore general-purpose registers from utf_regs

    ; Restore eflags from the stack.  After you do this, you can
    ; no longer use arithmetic operations or anything else that
    ; modifies eflags.
    ; LAB 4: Your code here.
    add  esp, 4             ; skip utf_eip
    popf                    ; restore eflags from the stack

    ; Switch back to the adjusted trap-time stack.
    ; LAB 4: Your code here.
    pop  esp                ; pop utf_esp into %esp

    ; Return to re-execute the instruction that faulted.
    ; LAB 4: Your code here.
    ret