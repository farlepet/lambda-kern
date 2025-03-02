MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)gic.o \
         $(MDIR)gtimer.o \
         $(MDIR)int.o \
         $(MDIR)intr.o

dirs-y := timer

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

