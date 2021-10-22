# Architecture-specific Makefile options for X86
CPU        = ia32
HW         = pc

CFLAGS    += -m32 -march=i586
LDARCH     = -melf_i386
LDFLAGS    = $(LDARCH) -T kernel/arch/x86/arch.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_ARCH__=PLATFORM_ARCH_X86 \
             -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_IA32 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_PC

ASFLAGS    = -m32


SRCS      += $(wildcard $(ARCHSRC)/*.c) $(wildcard $(ARCHSRC)/*/*.c) $(wildcard $(ARCHSRC)/*/*/*.c)
SRCS      += $(wildcard $(ARCHSRC)/*.s) $(wildcard $(ARCHSRC)/*/*.s) $(wildcard $(ARCHSRC)/*/*/*.s)

ifeq ($(EMBEDINITRD), 1)
$(BUILDDIR)/lambda.o: $(OBJS) $(BUILDDIR)/initrd.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -melf_i386 -r -o $@ $(OBJS) $(BUILDDIR)/initrd.o
else 
$(BUILDDIR)/lambda.o: $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) -melf_i386 -r -o $@ $(OBJS)
endif

$(BUILDDIR)/lambda.shared: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	$(Q) $(CC) -m32 -shared -o $@ $< -T kernel/arch/x86/arch.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(LD) $(LDFLAGS) -o $@ $<
