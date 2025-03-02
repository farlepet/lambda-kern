MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-$(CONFIG_X86_APIC) += $(MDIR)apic.o
obj-$(CONFIG_X86_APIC_TIMER) += $(MDIR)apictimer.o

cflags-$(CONFIG_X86_APIC) += -DCONFIG_X86_APIC
cflags-$(CONFIG_X86_APIC_TIMER) += -DCONFIG_X86_APIC_TIMER

