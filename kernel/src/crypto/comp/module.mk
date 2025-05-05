MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)comp.o
obj-$(CONFIG_CRYPTO_COMP_LZOP) += $(MDIR)lzop.o

cflags-$(CONFIG_CRYPTO_COMP_LZOP) += -DCONFIG_CRYPTO_COMP_LZOP

