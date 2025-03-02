MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := boot \
          init \
          intr

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

