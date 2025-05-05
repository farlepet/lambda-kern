MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-$(CONFIG_CRYPTO_COMP) := comp

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

