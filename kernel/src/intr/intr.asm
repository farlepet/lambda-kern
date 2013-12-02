global handle_int
global load_idt
global dummy_int


idtr DW 0 ; For limit storage
	 DD 0 ; For base storage

int_func DD 0 ; Temporary storage

load_idt:
	mov  eax, [esp + 4]
	mov  [idtr + 2], eax
	mov  ax, [esp + 8]
	mov  [idtr], ax 
	lidt [idtr]        ; Load the IDT pointer.
	ret


handle_int:   
	push eax
	mov eax, [esp + 8]
	mov [int_func], eax
	pop eax

	pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

	mov ax, ds               ; Lower 16-bits of eax = ds.
	push eax                 ; save the data segment descriptor

	mov ax, 0x10  ; load the kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, [int_func]
	call eax

	pop eax        ; reload the original data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa                     ; Pops edi,esi,ebp...
	sti
	iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP


dummy_int:
	push .subf
	jmp handle_int
  .subf:
	ret