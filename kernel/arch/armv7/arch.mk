# Architecture-specific Makefile options for armv7

CFLAGS    += -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/armv7/inc/ \
			 -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -DARCH_ARMV7 -O2 \
			 -pipe -g -fno-stack-protector
LDFLAGS    = -marmelf -T kernel/arch/$(ARCH)/arch.ld

CFLAGS    += -march=armv7

ASFLAGS    = -march=armv7

export ASFLAGS

link:   $(OBJS) CD/boot/grub/stage2_eltorito
	@echo -e "\033[33m  \033[1mBuilding ARMv7-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/$(ARCH); $(MAKE) CC=$(CC)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"

	@$(LD) $(LDFLAGS) -r -o lambda.o $(OBJS) kernel/arch/$(ARCH)/arch.a

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