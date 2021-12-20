# Architecture-specific Makefile options for armv7

# Default target:
CPU        = cortex_a9
HW         = arm_vexpress_a9

HWDIR      = $(MAINDIR)/kernel/arch/$(ARCH)/hw/$(HW)
HWSRC      = $(HWDIR)/src
HWINC      = $(HWDIR)/inc

CFLAGS    += -marm -mapcs-frame -mpoke-function-name
LDARCH     = -marmelf
LDFLAGS    = -marm

ASFLAGS    = -marm

CFLAGS    += -D__LAMBDA_PLATFORM_ARCH__=PLATFORM_ARCH_ARM32 \
             -I$(HWINC)

ifneq ($(CC), clang)
CFLAGS  += -Wno-unknown-pragmas
endif

# TODO: Allow selecting of specific source files in a smart way
SRCS      += $(wildcard $(HWSRC)/*.c)   $(wildcard $(HWSRC)/*/*.c)   $(wildcard $(HWSRC)/*/*/*.c)
SRCS      += $(wildcard $(HWSRC)/*.s)   $(wildcard $(HWSRC)/*/*.s)   $(wildcard $(HWSRC)/*/*/*.s)
SRCS      += $(wildcard $(ARCHSRC)/*.c) $(wildcard $(ARCHSRC)/*/*.c) $(wildcard $(ARCHSRC)/*/*/*.c)
SRCS      += $(wildcard $(ARCHSRC)/*.s) $(wildcard $(ARCHSRC)/*/*.s) $(wildcard $(ARCHSRC)/*/*/*.s)

include $(HWDIR)/hw.mk

ifeq ($(EMBEDINITRD), 1)
$(BUILDDIR)/lambda.o: $(OBJS) $(BUILDDIR)/initrd.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -r -o $@ $(OBJS) $(BUILDDIR)/initrd.o
else 
$(BUILDDIR)/lambda.o: $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -r -o $@ $(OBJS)
endif
