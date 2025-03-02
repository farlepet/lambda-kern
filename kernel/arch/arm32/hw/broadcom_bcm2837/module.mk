# HW-specific Makefile options for the Broadcom BCM2837

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

CPU        = cortex_a53

KERNEL_OFFSET = 0x00000000

cflags-y  += -march=armv7-a -marm \
             -DKERNEL_OFFSET=${KERNEL_OFFSET}

cflags-y  += -DCONFIG_ARCH_CPU_CORTEX_A53 \
             -DCONFIG_ARCH_HW_BROADCOM_BCM2837

asflags-y += -march=armv7-a

ifeq ($(CONFIG_BUILD_USE_CLANG),y)
    cflags-y  += -target armv8--eabi -mcpu=cortex-a53
    asflags-y += -target armv8--eabi -mcpu=cortex-a53
endif

include $(MDIR)src/module.mk

.DEFAULT_GOAL=$(BUILDDIR)/kernel7.img

$(BUILDDIR)/kernel7.img: $(BUILDDIR)/lambda.kern
	@echo -e "\033[33m  \033[1mProducing RPi binary\033[0m"
	$(Q) $(OBJCOPY) $< -O binary $@


$(BUILDDIR)/lambda.ukern: $(BUILDDIR)/lambda.kern
	@echo -e "\033[33m  \033[1mGenerating U-Boot compatible kernel image\033[0m"
	$(Q) mkimage -A arm -C none -T kernel -a 0x00008000 -r 0x00008000 -d $< $@


#@qemu-system-aarch64 -M raspi3 -kernel $< -serial stdio -no-reboot
emu: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -M raspi2b -kernel $< -serial stdio -no-reboot

#@qemu-system-aarch64 -M raspi3 -kernel $< -serial stdio -no-reboot -s -S
emu-debug: $(BUILDDIR)/lambda.kern
	@qemu-system-arm -M raspi2b -kernel $< -serial stdio -no-reboot -s -S

