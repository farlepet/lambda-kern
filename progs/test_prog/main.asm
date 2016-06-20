main:
	mov eax, 0
	mov ebx, int_req
	int 0xFF
	
	mov eax, 1
	mov ebx, int_req
	mov DWORD [int_req + 4], kvid_req
	
	jmp main


data:

str1 dd "Hello, World!", 0

int_req	dd 0
		dd 0
		dd 0

kvid_req	dd 0    ; pid unknown...
			db 0    ; print
			dd str1 ; string