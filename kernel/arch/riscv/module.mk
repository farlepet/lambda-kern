# Architecture-specific Makefile options for riscv

cflags-y  += 
ldflags-y += -T kernel/arch/$(ARCH)/kendryte-k210.ld

cflags-y  += -DCONFIG_ARCH_RISCV \
             -DCONFIG_ARCH_CPU_RISCV_RV64I \
             -DCONFIG_ARCH_HW_KENDRYTE_K210

