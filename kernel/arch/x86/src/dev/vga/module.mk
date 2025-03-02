MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)print.o

