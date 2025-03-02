MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)stack.o \
         $(MDIR)stack_trace.o \
         $(MDIR)syscall.o \
         $(MDIR)tasking.o \
         $(MDIR)user.o

