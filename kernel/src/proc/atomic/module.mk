MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)lock.o \
         $(MDIR)tlock.o

