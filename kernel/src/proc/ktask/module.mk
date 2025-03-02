MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)kbug.o \
         $(MDIR)kinput.o \
         $(MDIR)ktasks.o \
         $(MDIR)kterm.o
