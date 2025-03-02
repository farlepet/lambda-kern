MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)apic.o \
         $(MDIR)apictimer.o

