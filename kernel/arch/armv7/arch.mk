# Architecture-specific Makefile options for armv7

CFLAGS    += -DARCH_ARMV7 -march=armv7-a -marm
LDARCH     = -marmelf
LDFLAGS    = -marm -T kernel/arch/$(ARCH)/qemu-vexpress-a9.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_ARCH__=PLATFORM_ARCH_ARMV7 \
             -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_ARM_CORTEX_A9 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_ARM_VEXPRESS_A9

ASFLAGS    = -march=armv7-a -marm

ifeq ($(CC), clang)
CFLAGS  += -target armv7--eabi -mcpu=cortex-a9
ASFLAGS += -target armv7--eabi -mcpu=cortex-a9
else
CFLAGS  += -Wno-unknown-pragmas
endif


ifeq ($(EMBEDINITRD), 1)
$(BUILDDIR)/lambda.o: $(OBJS) $(BUILDDIR)/initrd.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -r -o $@ $(OBJS) $(BUILDDIR)/initrd.o
else 
$(BUILDDIR)/lambda.o: $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -r -o $@ $(OBJS)
endif

$(BUILDDIR)/lambda.shared: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	$(Q) $(CC) -shared -o $@ $< -T kernel/arch/x86/arch.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lgcc



$(BUILDDIR)/lambda.ukern: $(BUILDDIR)/lambda.kern
	@echo -e "\033[33m  \033[1mGenerating U-Boot compatible kernel image\033[0m"
	$(Q) mkimage -A arm -C none -T kernel -a 0x60000000 -r 0x60000000 -d $< $@

$(BUILDDIR)/sd.img: $(BUILDDIR)/lambda.ukern
	@echo -e "\033[33m  \033[1mGenerating SD card image\033[0m"
	# TODO: Find a way to do this without resorting to SUDO
	kernel/arch/armv7/sdimg.sh


emu: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -cpu cortex-a9 -machine vexpress-a9 -kernel $< -serial stdio -no-reboot

emu-debug: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -cpu cortex-a9 -machine vexpress-a9 -kernel $< -serial stdio -no-reboot -s -S
