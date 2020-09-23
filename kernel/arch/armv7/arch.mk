# Architecture-specific Makefile options for armv7

CFLAGS    += -DARCH_ARMV7 -march=armv7
LDFLAGS    = -T kernel/arch/$(ARCH)/arch.ld

ASFLAGS    = -march=armv7

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

initrd.cpio:
	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../initrd.cpio &> /dev/null


emu:
	@qemu-system-arm -cpu cortex-a7 -machine versatilepb -kernel lambda.kern -serial stdio -no-reboot

emu-debug:
	@qemu-system-arm -cpu cortex-a7 -machine versatilepb -kernel lambda.kern -serial stdio -no-reboot -s -S

arch_clean:
	@rm -f common.o lambda.o arch.a symbols.o initrd.cpio lambda.kern symbols.c


common_msg:
	@echo -e "\033[33m  \033[1mBuilding common\033[0m"

arch_msg:
	@echo -e "\033[33m  \033[1mBuilding ARMv7-specific code\033[0m"
