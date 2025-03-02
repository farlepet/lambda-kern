# HW-specific Makefile options for ARM VExpress A9

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

CPU        = cortex_a9

KERNEL_OFFSET = 0x00000000

cflags-y  += -march=armv7-a \
             -DKERNEL_OFFSET=${KERNEL_OFFSET}
ldflags-y += -T $(HWDIR)/hw.ld

# TODO: Make this command-line selectable:
cflags-y  += -DCONFIG_ARCH_CPU_ARM_CORTEX_A9 \
             -DCONFIG_ARCH_HW_ARM_VEXPRESS_A9

asflags-y += -march=armv7-a

ifeq ($(CONFIG_BUILD_USE_CLANG),y)
    cflags-y  += -target armv7--eabi -mcpu=cortex-a9
    asflags-y += -target armv7--eabi -mcpu=cortex-a9
endif

include $(MDIR)src/module.mk


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
