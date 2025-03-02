MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)boot.o \
         $(MDIR)entry.o

