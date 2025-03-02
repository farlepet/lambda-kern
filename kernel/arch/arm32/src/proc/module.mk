
MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)stack.o \
         $(MDIR)stacktrace.o \
         $(MDIR)syscall.o \
         $(MDIR)tasking.o
