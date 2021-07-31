# Architecture-specific Makefile options for riscv

CFLAGS    += -DARCH_RISCV
LDFLAGS    = -T kernel/arch/$(ARCH)/kendryte-k210.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_ARCH__=PLATFORM_ARCH_RISCV \
             -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_RISCV_RV64I \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_KENDRYTE_K210

ASFLAGS    = 


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
	$(Q) $(CC) -shared -o $@ $< -T kernel/arch/x86/arch.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lgcc
