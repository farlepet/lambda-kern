MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := boot \
          init

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

