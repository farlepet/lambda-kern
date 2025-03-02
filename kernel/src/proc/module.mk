MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)atomic.o \
         $(MDIR)cond.o \
         $(MDIR)exec.o \
         $(MDIR)fork.o \
         $(MDIR)mtask.o \
         $(MDIR)proc.o \
         $(MDIR)sched.o \
         $(MDIR)syscalls.o \
         $(MDIR)thread.o \
         $(MDIR)wait.o

dirs-y := elf \
          ktask

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

cflags-$(CONFIG_KTASK_DEBUGGER) += -DCONFIG_KTASK_DEBUGGER
