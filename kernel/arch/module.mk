MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CONFIG_ARCH_X86),y)
	include $(MDIR)x86/module.mk
else ifeq ($(CONFIG_ARCH_ARM32),y)
	include $(MDIR)arm32/module.mk
else ifeq ($(CONFIG_ARCH_RISCV),y)
	include $(MDIR)riscv/module.mk
endif

#cflags-$(CONFIG_VERBOSE_PANIC) += -DCONFIG_VERBOSE_PANIC

