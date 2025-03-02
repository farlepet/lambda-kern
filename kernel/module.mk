# Kernel source root

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := arch \
          src

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

#cflags-$(CONFIG_VERBOSE_PANIC) += -DCONFIG_VERBOSE_PANIC

