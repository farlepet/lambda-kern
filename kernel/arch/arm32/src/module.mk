MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := init \
          intr \
          io   \
          mm   \
          proc

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

