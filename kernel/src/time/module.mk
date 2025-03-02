MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)delay.o \
         $(MDIR)time.o

