MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)input.o

