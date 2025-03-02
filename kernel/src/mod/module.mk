MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)module.o \
         $(MDIR)preload.o \
         $(MDIR)symbols.o
