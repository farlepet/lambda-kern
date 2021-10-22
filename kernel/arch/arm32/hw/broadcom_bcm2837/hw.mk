# Architecture-specific Makefile options for the Broadcom BCM2837

CFLAGS    += -march=armv8-a -marm
LDFLAGS   += -T $(HWDIR)/hw.ld

CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_ARM_CORTEX_A53 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_BROADCOM_BCM2837

ASFLAGS    = -march=armv8-a

ifeq ($(CC), clang)
CFLAGS  += -target armv8--eabi -mcpu=cortex-a53
ASFLAGS += -target armv8--eabi -mcpu=cortex-a53
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
	$(Q) $(CC) -shared -o $@ $< -T kernel/arch/arm32/arch.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lgcc



$(BUILDDIR)/lambda.ukern: $(BUILDDIR)/lambda.kern
	@echo -e "\033[33m  \033[1mGenerating U-Boot compatible kernel image\033[0m"
	$(Q) mkimage -A arm -C none -T kernel -a 0x00080000 -r 0x00080000 -d $< $@


emu: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -M raspi3 -kernel $< -serial stdio -no-reboot

emu-debug: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -M raspi3 -kernel $< -serial stdio -no-reboot -s -S

