# Architecture-specific Makefile options for X86

CFLAGS    += -m32 -DARCH_X86 -O2 -march=i586
LDFLAGS    = -melf_i386 -T kernel/arch/x86/arch.ld

ASFLAGS    = -m32

export ASFLAGS


link:   $(OBJS) CD/boot/grub/stage2_eltorito
	@echo -e "\033[33m  \033[1mBuilding x86-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/x86; $(MAKE)

	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(LD) $(LDFLAGS) -r -o lambda.o $(OBJS) kernel/arch/x86/arch.a

	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	@scripts/symbols > symbols.c
	@$(CC) $(CFLAGS) -c -o symbols.o symbols.c

	@$(LD) $(LDFLAGS) -o lambda.kern lambda.o symbols.o
	#@cp lambda.kern CD/lambda.kern
	@strip lambda.kern -o CD/lambda.kern

	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../CD/initrd.cpio &> /dev/null
	@echo -e "\033[33m  \033[1mCreating ISO\033[0m"

	@grub-mkrescue -o lambda-os.iso CD

#	@genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 \
#		-boot-info-table -o lambda-os.iso CD


#CD/boot/grub/stage2_eltorito:
#	@echo -e "\033[33m	\033[1mDownloading GRUB stage 2 binary\033[0m"
#	@curl -o CD/boot/grub/stage2_eltorito https://arabos.googlecode.com/files/stage2_eltorito


emu:
	@qemu-system-x86_64 -cdrom lambda-os.iso -serial stdio -machine pc -no-reboot