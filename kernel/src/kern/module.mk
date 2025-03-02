MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)cmdline.o

