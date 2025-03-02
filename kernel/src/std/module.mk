MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)libgen.o \
         $(MDIR)stdlib.o \
         $(MDIR)string.o
