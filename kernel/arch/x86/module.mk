# Architecture-specific Makefile options for X86

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

ARCH      := x86

ifeq ($(CONFIG_ARCH_CPU_IA32),y)
    CPU := ia32
endif

ifeq ($(CONFIG_ARCH_HW_PC),y)
    HW := pc
endif

HWDIR     := $(MDIR)hw/$(HW)
HWINC     := $(HWDIR)/inc

LDARCH     =
ldflags-y += $(LDARCH)

cflags-y  += -DCONFIG_ARCH_X86 \
             -I$(HWINC)

ifeq ($(CONFIG_BUILD_USE_CLANG),y)
    cflags-y  += -target i486
    asflags-y += -target i486
    ldoflags-y += -melf_i386
    ldkflags-y += -target i486 -static
endif

include $(MDIR)src/module.mk
include $(HWDIR)/module.mk


