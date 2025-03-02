MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)mmu.o

