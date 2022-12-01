# Architecture-specific Makefile options for X86

# Default target:
CPU        = ia32
HW         = pc

HWDIR      = $(MAINDIR)/kernel/arch/$(ARCH)/hw/$(HW)
HWSRC      = $(HWDIR)/src
HWINC      = $(HWDIR)/inc

CFLAGS    +=
LDARCH     =
LDFLAGS   += $(LDARCH)

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_ARCH__=PLATFORM_ARCH_X86 \
             -I$(HWINC)

ASFLAGS   +=

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

$(BUILDDIR)/lambda.shared: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	$(Q) $(CC) -shared -o $@ $< -T $(HWDIR)/hw.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(CC) -o $@ $< -T $(HWDIR)/hw.ld -nostdlib -lgcc

