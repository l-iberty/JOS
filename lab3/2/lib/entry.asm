[SECTION .data]
; Define the global symbols 'envs', 'pages', 'uvpt', and 'uvpd'
; so that they can be used in C as if they were ordinary global arrays.
global envs
    envs equ ENVS
global pages
    pages equ UPAGES
global uvpt
    uvpt equ UVPT
global uvpd
    uvpd equ (UVPT+(UVPT>>12)*4)

ENVS        equ 1024
UPAGES      equ 0xEF000000
UVPT        equ 0xEF400000
USTACKTOP   equ 0xEEBFE000

; Entrypoint - this is where the kernel (or our parent environment)
; starts us running when we are initially loaded into a new environment.
[SECTION .text]
global _start
_start:
    ; See if we were started with arguments on the stack
    cmp esp, USTACKTOP
    jne .args_exist

    ; If not, push dummy argc/argv arguments.
    ; This happens when we are loaded by the kernel,
    ; because the kernel does not know about passing arguments.
    push dword 0
    push dword 0

.args_exist:
extern libmain
    call libmain
    jmp $