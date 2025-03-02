MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := acpi \
          boot  \
          dev   \
          init \
          intr   \
          io \
          mm \
          proc

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

