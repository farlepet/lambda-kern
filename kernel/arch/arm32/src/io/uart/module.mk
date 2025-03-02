MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)uart_allwinner.o \
         $(MDIR)uart_pl011.o
