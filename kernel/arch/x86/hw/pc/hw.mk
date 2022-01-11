# HW-specific Makefile options for X86-based PCs

HW         = pc

CFLAGS    += -march=i586
LDFLAGS   += -T $(HWDIR)/hw.ld

CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_IA32 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_PC

ASFLAGS   += -march=i586

.DEFAULT_GOAL=$(BUILDDIR)/lambda.kern
