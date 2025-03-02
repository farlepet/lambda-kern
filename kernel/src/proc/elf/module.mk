MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)elf.o \
         $(MDIR)elf_exec.o
