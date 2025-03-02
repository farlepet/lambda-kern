MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)gdt.o \
         $(MDIR)mm.o \
         $(MDIR)mmu.o \
         $(MDIR)paging.o

