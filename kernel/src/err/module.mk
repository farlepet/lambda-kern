MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)error.o \
         $(MDIR)panic.o

