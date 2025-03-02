MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)bcm2835_systimer.o

