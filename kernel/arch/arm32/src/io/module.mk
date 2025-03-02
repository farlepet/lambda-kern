MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := uart

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

