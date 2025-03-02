MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)hw_init.o
