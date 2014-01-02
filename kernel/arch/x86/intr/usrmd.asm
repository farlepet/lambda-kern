global run_usr

run_usr:
     mov ax,0x23 ; Data Segment
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax
 
     mov eax, [esp + 4]
     push 0x23 ; Data segment
     push esp
     pushf
     push 0x1B ; Code segment
     push eax  ; Location to jump to
     iret