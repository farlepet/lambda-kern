# HW-specific Makefile options for X86-based PCs

HW         = pc

KERNEL_OFFSET = 0xC0000000

CFLAGS    += -DKERNEL_OFFSET=${KERNEL_OFFSET}
LDFLAGS   += -T $(HWDIR)/hw.ld

CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_IA32 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_PC

ASFLAGS   +=

.DEFAULT_GOAL=$(BUILDDIR)/lambda.kern
