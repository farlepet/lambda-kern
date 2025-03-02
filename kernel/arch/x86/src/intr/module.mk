MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)cexceptions.o \
         $(MDIR)exceptions.o \
         $(MDIR)idt.o \
         $(MDIR)intr.o \
         $(MDIR)pic.o \
         $(MDIR)pit.o

dirs-y := apic

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

