MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)cond.o \
         $(MDIR)exec.o \
         $(MDIR)fork.o \
         $(MDIR)mtask.o \
         $(MDIR)proc.o \
         $(MDIR)sched.o \
         $(MDIR)syscalls.o \
         $(MDIR)thread.o \
         $(MDIR)wait.o

dirs-y := atomic \
          elf \
          ktask

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

cflags-y += -DCONFIG_PROC_KERN_STACK_SIZE=$(CONFIG_PROC_KERN_STACK_SIZE) \
            -DCONFIG_PROC_USER_STACK_SIZE=$(CONFIG_PROC_USER_STACK_SIZE) \
            -DCONFIG_PROC_USER_STACK_SIZE_MAX=$(CONFIG_PROC_USER_STACK_SIZE_MAX)

cflags-$(CONFIG_PROC_STACK_GUARD) += -DCONFIG_PROC_STACK_GUARD
cflags-$(CONFIG_KTASK_DEBUGGER)   += -DCONFIG_KTASK_DEBUGGER

