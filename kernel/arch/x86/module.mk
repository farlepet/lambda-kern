# Architecture-specific Makefile options for X86

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

ARCH      := x86

ifeq ($(CONFIG_ARCH_CPU_386),y)
    CPU := i386
    cflags-y += -DCONFIG_ARCH_CPU_386
else ifeq ($(CONFIG_ARCH_CPU_486),y)
    CPU := i486
    cflags-y += -DCONFIG_ARCH_CPU_386
else ifeq ($(CONFIG_ARCH_CPU_P5),y)
    CPU := p5
    cflags-y += -DCONFIG_ARCH_CPU_P5
else ifeq ($(CONFIG_ARCH_CPU_P6),y)
    CPU := p6
    cflags-y += -DCONFIG_ARCH_CPU_P6
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
    # TODO: Select based on CPU selection, to (presumably) enable more
    # optimizations
    cflags-y  += -target i486
    asflags-y += -target i486
    ldoflags-y += -melf_i386
    ldkflags-y += -target i486 -static
endif

include $(MDIR)src/module.mk
include $(HWDIR)/module.mk


