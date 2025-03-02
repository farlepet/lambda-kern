# Architecture-specific Makefile options for armv7

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

ARCH      := arm32

ifeq ($(CONFIG_ARCH_CPU_CORTEX_A7),y)
    CPU := cortex_a7
else ifeq ($(CONFIG_ARCH_CPU_CORTEX_A9),y)
    CPU := cortex_a9
else ifeq ($(CONFIG_ARCH_CPU_CORTEX_A53),y)
    CPU := cortex_a53
endif

ifeq ($(CONFIG_ARCH_HW_ARM_VEXPRESS_A9),y)
    HW := arm_vexpress_a9
else ifeq ($(CONFIG_ARCH_HW_ALLWINNER_V3S),y)
    HW := arm_allwinner_v3s
else ifeq ($(CONFIG_ARCH_HW_BROADCOM_BCM2837),y)
    HW := broadcom_bcm2837
endif

HWDIR     := $(MDIR)hw/$(HW)
HWINC     := $(HWDIR)/inc

cflags-y  += -marm
LDARCH     = -marmelf
ldflags-y += -marm -nostartfiles

asflags-y += -marm

cflags-y  += -DCONFIG_ARCH_ARM32 \
             -I$(HWINC)

ifneq ($(CONFIG_BUILD_USE_CLANG),y)
    cflags-y += -Wno-unknown-pragmas  \
                -mapcs-frame -mpoke-function-name
endif

include $(MDIR)src/module.mk
include $(HWDIR)/module.mk

