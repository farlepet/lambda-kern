MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)timer_sp804.o

