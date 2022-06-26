# Memory Management related functions

.global load_gdt
.global seg_reload
.global load_tss

# Load the new GDT
load_gdt:
	movl   4(%esp), %eax
	movl   %eax, (gdtr+2)
	movw   8(%esp), %ax
	movw   %ax, (gdtr)
	lgdt   (gdtr)
	ret

# Reload various segments to allow the GDT to take effect
seg_reload:
	jmp   $0x08,$.reload_cs
.reload_cs:
	movw   $0x10, %ax
	movw   %ax, %ds
	movw   %ax, %es
	movw   %ax, %fs
	movw   %ax, %gs
	movw   %ax, %ss
	ret

# Let the computer know which GDT descriptor contains TSS info
load_tss:
   movw $0x4B, %ax
   ltr %ax
   ret

.section .data
gdtr: .word 0 # For limit storage
	  .long 0 # For base storage
