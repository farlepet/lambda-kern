MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += $(MDIR)fs.o \
         $(MDIR)initrd.o \
         $(MDIR)procfs.o \
         $(MDIR)stat.o \
         $(MDIR)stream.o

