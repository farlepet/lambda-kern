; Enter a specified ring
global enter_ring
enter_ring:
    mov ebx, [esp+4] ; Ring to enter
    mov ecx, [esp+8] ; Address to jump to
    ;mov ecx, eax

    ; Get desired segment selector:
    and ebx, 0x03
    ;mov eax, 0x20 ; User data segment
    ;or  ax, bx
    ;mov eax, 0x23
    mov eax, [data_selectors + (ebx * 4)]

    ; Set selectors:
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov edx, esp
    push eax ; User data segment
    push edx ; Stack pointer
    
    pushf

    mov eax, [code_selectors + (ebx * 4)];0x18 ; User code segment
    ;or  ax, bx
    push eax
    ;push 0x1B

    push ecx ; Address to jump to
    
    ; Stack before iret:
    ;  EIP
    ;  CS
    ;  EFLAGS
    ;  ESP
    ;  DS
    iret

    ; SYSCALL_EXIT
    mov eax, 3
    mov ebx, exit_data
    int 0xFF

.loop: jmp .loop



exit_data:
    dd 0x01


code_selectors:
    dd 0x08
    dd 0x19
    dd 0x2A
    dd 0x3B

data_selectors:
    dd 0x10
    dd 0x21
    dd 0x32
    dd 0x43
