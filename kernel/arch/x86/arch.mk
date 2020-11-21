# Architecture-specific Makefile options for X86

CFLAGS    += -m32 -DARCH_X86 -O2 -march=i586
LDFLAGS    = -melf_i386 -T kernel/arch/x86/arch.ld

ASFLAGS    = -m32

export ASFLAGS

arch_all: lambda-os.iso

arch.a: arch_msg
	@cd $(MAINDIR)/kernel/arch/$(ARCH); $(MAKE) CC=$(CC)
	@cp $(MAINDIR)/kernel/arch/$(ARCH)/arch.a ./arch.a

common.o: common_msg $(OBJS)
	@echo -e "\033[33m  \033[1mLinking common\033[0m"
	@$(LD) -melf_i386 -r -o common.o $(OBJS)

lambda.o: arch.a common.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(LD) -melf_i386 -r -o lambda.o common.o arch.a

symbols.o: lambda.o
	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	@scripts/symbols > symbols.c
	@$(CC) $(CFLAGS) -c -o symbols.o symbols.c

lambda.lib: lambda.o symbols.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(LD) $(LDFLAGS) -r -o lambda.lib lambda.o symbols.o

lambda.shared: lambda.lib
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(CC) -m32 -shared -o lambda.shared lambda.lib

lambda.kern: lambda.lib
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	@$(LD) $(LDFLAGS) -o lambda.kern lambda.lib

initrd.cpio:
	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../initrd.cpio &> /dev/null

lambda-os.iso: lambda.kern initrd.cpio CD/boot/grub/stage2_eltorito
	@strip lambda.kern -o CD/lambda.kern
	@cp initrd.cpio CD/
	@grub-mkrescue -o lambda-os.iso CD

#	@genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 \
#		-boot-info-table -o lambda-os.iso CD


#CD/boot/grub/stage2_eltorito:
#	@echo -e "\033[33m	\033[1mDownloading GRUB stage 2 binary\033[0m"
#	@curl -o CD/boot/grub/stage2_eltorito https://arabos.googlecode.com/files/stage2_eltorito


emu:
	@qemu-system-x86_64 -cdrom lambda-os.iso -serial stdio -machine pc -no-reboot

arch_clean:
	@rm -f common.o lambda.o arch.a symbols.o initrd.cpio lambda.kern symbols.c lambda-os.iso


common_msg:
	@echo -e "\033[33m  \033[1mBuilding common\033[0m"

arch_msg:
	@echo -e "\033[33m  \033[1mBuilding ARMv7-specific code\033[0m"
