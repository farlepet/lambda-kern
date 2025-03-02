MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)boot.o \
         $(MDIR)entry.o \
         $(MDIR)multiboot.o

cflags-$(CONFIG_MULTIBOOT) += -DCONFIG_MULTIBOOT \
                              -DCONFIG_MULTIBOOT_VERSION=$(CONFIG_MULTIBOOT_VERSION)
