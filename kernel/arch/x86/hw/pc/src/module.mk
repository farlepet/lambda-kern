MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := init

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

