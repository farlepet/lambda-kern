# Architecture-specific Makefile options for armv7

CFLAGS    += -DARCH_ARMV7 -march=armv7-a -marm
LDFLAGS    = -marm -T kernel/arch/$(ARCH)/qemu-vexpress-a9.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_ARM_CORTEX_A9 -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_ARM_VEXPRESS_A9

ASFLAGS    = -march=armv7-a -marm

export ASFLAGS

arch_all: lambda.kern initrd.cpio

arch.a: arch_msg
	@cd $(MAINDIR)/kernel/arch/$(ARCH); $(MAKE) CC=$(CC)
	@cp $(MAINDIR)/kernel/arch/$(ARCH)/arch.a ./arch.a

common.o: common_msg $(OBJS)
	@echo -e "\033[33m  \033[1mLinking common\033[0m"
	@$(LD) -r -o common.o $(OBJS)

lambda.o: arch.a common.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(LD) -r -o lambda.o common.o arch.a

symbols.o: lambda.o
	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	@scripts/symbols > symbols.c
	@$(CC) $(CFLAGS) -c -o symbols.o symbols.c

lambda.kern: lambda.o symbols.o arch.a
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o lambda.kern lambda.o symbols.o arch.a -lgcc

lambda.ukern: lambda.kern
	@echo -e "\033[33m  \033[1mGenerating U-Boot compatible kernel image\033[0m"
	@mkimage -A arm -C none -T kernel -a 0x60000000 -r 0x60000000 -d lambda.kern lambda.ukern

sd.img: lambda.ukern
	@echo -e "\033[33m  \033[1mGenerating SD card image\033[0m"
	# TODO: Find a way to do this without resorting to SUDO
	kernel/arch/armv7/sdimg.sh	

initrd.cpio:
	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../initrd.cpio &> /dev/null


emu: lambda.kern
	@qemu-system-arm -cpu cortex-a9 -machine vexpress-a9 -kernel lambda.kern -serial stdio -no-reboot

emu-debug: lambda.kern
	@qemu-system-arm -cpu cortex-a9 -machine vexpress-a9 -kernel lambda.kern -serial stdio -no-reboot -s -S -d int

arch_clean:
	@rm -f common.o lambda.o arch.a symbols.o initrd.cpio lambda.kern symbols.c


common_msg:
	@echo -e "\033[33m  \033[1mBuilding common\033[0m"

arch_msg:
	@echo -e "\033[33m  \033[1mBuilding ARMv7-specific code\033[0m"
