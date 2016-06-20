org 0x80000000

mov eax, 0x00        ; get ktask pid
mov ebx, ktask_req
int 0xFF             ; syscall

mov eax, [ktask_req] ; returned ktask pid
mov [kvid_msg], eax

mov eax, 0x01
mov ebx, kvid_msg
int 0xFF

ret

;t:
;jmp t

ktask_req:
	dd 0x01 ; kvid
	dd 0x00

kvid_msg:
	dd 0x00 ; kvid pid
	dd kvid_data ; message
	dd 9 ; message size
	
kvid_data:
	dd 0x00 ; sender pid [dontcare]
	db "TEST", 0x00
