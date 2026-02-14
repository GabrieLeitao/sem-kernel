[global jump_to_user_mode]
jump_to_user_mode:
    mov ax, 0x23 ; User data segment selector with RPL 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push 0x23 ; SS
    push eax  ; ESP
    pushf     ; EFLAGS
    pop eax
    or eax, 0x200 ; Enable interrupts in user mode
    push eax
    push 0x1B ; CS (User code segment selector with RPL 3)
    push .user_start ; EIP
    iret

.user_start:
    ; We are now in user mode!
    mov eax, 0 ; Syscall 0: print
    mov ebx, .msg
    int 0x80
    
    mov eax, 1 ; Syscall 1: exit
    int 0x80
    jmp $ ; Should not be reached

.msg db "Hello from User Mode!", 0x0A, 0
