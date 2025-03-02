# HW-specific Makefile options for X86-based PCs

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

HW        := pc

KERNEL_OFFSET = 0xC0000000

cflags-y    += -DKERNEL_OFFSET=${KERNEL_OFFSET}
ldflags-y   += -T $(HWDIR)/hw.ld

cflags-y    += -DCONFIG_ARCH_HW_PC

asflags-y   +=

.DEFAULT_GOAL=$(BUILDDIR)/lambda.kern

include $(MDIR)src/module.mk

