# Common kernel source

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := data \
          err  \
          fs   \
          intr \
          io   \
          kern \
          main \
          mm   \
          mod  \
          proc \
          std  \
          time

dirs-$(CONFIG_CRYPTO) += crypto

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

cflags-$(CONFIG_LOG_COLORCODE)   += -DCONFIG_LOG_COLORCODE
cflags-$(CONFIG_EMBEDDED_INITRD) += -DCONFIG_EMBEDDED_INITRD
cflags-$(CONFIG_SAFETY_CHECKS)   += -DCONFIG_SAFETY_CHECKS

cflags-y += -DCONFIG_STRICTNESS=$(CONFIG_STRICTNESS)

