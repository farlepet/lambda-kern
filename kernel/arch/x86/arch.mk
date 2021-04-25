# Architecture-specific Makefile options for X86

CFLAGS    += -m32 -DARCH_X86 -O2 -march=i586
LDARCH     = -melf_i386
LDFLAGS    = $(LDARCH) -T kernel/arch/x86/arch.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_X86 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_PC

ASFLAGS    = -m32

export ASFLAGS

FLOPPY     = lambda-os.img

arch_all: lambda-os.iso


common.o: common_msg $(OBJS)
	@echo -e "\033[33m  \033[1mLinking common\033[0m"
	@$(LD) -melf_i386 -r -o common.o $(OBJS)

lambda.o: arch.a common.o initrd.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(LD) -melf_i386 -r -o lambda.o common.o arch.a initrd.o


lambda.lib: lambda.o symbols.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(LD) $(LDFLAGS) -r -o lambda.lib lambda.o symbols.o

lambda.shared: lambda.lib
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(CC) -m32 -shared -o lambda.shared lambda.lib

lambda.kern: lambda.lib
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	@$(LD) $(LDFLAGS) -o lambda.kern lambda.lib

lambda-os.iso: lambda.kern initrd.cpio CD/boot/grub/stage2_eltorito
	@strip lambda.kern -o CD/lambda.kern
	@cp initrd.cpio CD/
	@grub-mkrescue -o lambda-os.iso CD

$(FLOPPY): lambda.kern initrd.cpio
	rm -f $(FLOPPY)
	mkdosfs -C $(FLOPPY) 1440
	mcopy -i $(FLOPPY) syslinux.cfg ::/
	mcopy -i $(FLOPPY) /usr/lib/syslinux/bios/mboot.c32 ::/
	mcopy -i $(FLOPPY) /usr/lib/syslinux/bios/libcom32.c32 ::/
	mcopy -i $(FLOPPY) initrd.cpio ::/
	strip lambda.kern -o lambda.kern.stripped
	mcopy -i $(FLOPPY) lambda.kern.stripped ::/lambda.kern
	rm -f lambda.kern.stripped
	syslinux -i $(FLOPPY)

emu:
	@qemu-system-i386 -cdrom lambda-os.iso -serial stdio -machine pc -no-reboot

emu-debug:
	@qemu-system-i386 -cdrom lambda-os.iso -serial stdio -machine pc -no-reboot -gdb tcp::1234 -S

arch_clean:
	@rm -f common.o lambda.o arch.a lambda.kern lambda-os.iso


common_msg:
	@echo -e "\033[33m  \033[1mBuilding common\033[0m"

arch_msg:
	@echo -e "\033[33m  \033[1mBuilding x86-specific code\033[0m"
