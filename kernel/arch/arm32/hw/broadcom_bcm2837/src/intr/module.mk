MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)bcm2835_intctlr.o

dirs-y := timer

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

