MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := keyb \
          vga

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

