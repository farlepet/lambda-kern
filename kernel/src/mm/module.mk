MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)alloc.o \
         $(MDIR)mm.o \
         $(MDIR)mmap.o \
         $(MDIR)mmu.o \
         $(MDIR)symbols.o

