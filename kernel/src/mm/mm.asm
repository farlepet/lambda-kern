; Memory Management related functions

global load_gdt
global seg_reload
global load_tss

gdtr DW 0 ; For limit storage
	 DD 0 ; For base storage

; Load the new GDT
load_gdt:
	mov   eax, [esp + 4]
	mov   [gdtr + 2], eax
	mov   ax, [esp + 8]
	mov   [gdtr], ax
	lgdt  [gdtr]
	ret

; Reload various segments to allow the GDT to take effect
seg_reload:
	JMP   0x08:.reload_cs
.reload_cs:
	mov   ax, 0x10
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	ret

; Let the computer know which GDT descriptor contains TSS info
load_tss:
   mov ax, 0x2B
   ltr ax
   ret