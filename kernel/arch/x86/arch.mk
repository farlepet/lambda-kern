# Architecture-specific Makefile options for X86

CFLAGS    += -m32 -DARCH_X86 -O2 -march=i586
LDARCH     = -melf_i386
LDFLAGS    = $(LDARCH) -T kernel/arch/x86/arch.ld

# TODO: Make this command-line selectable:
CFLAGS    += -D__LAMBDA_PLATFORM_CPU__=PLATFORM_CPU_X86 \
             -D__LAMBDA_PLATFORM_HW__=PLATFORM_HW_PC

ASFLAGS    = -m32

export ASFLAGS

arch_all: lambda.kern


common.o: common_msg $(OBJS)
	@echo -e "\033[33m  \033[1mLinking common\033[0m"
	@$(LD) -melf_i386 -r -o common.o $(OBJS)

lambda.o: arch.a common.o initrd.o
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(LD) -melf_i386 -r -o lambda.o common.o arch.a initrd.o

lambda.shared: lambda.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	@$(CC) -m32 -shared -o lambda.shared lambda.o -T kernel/arch/x86/arch.ld

lambda.kern: lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	@$(LD) $(LDFLAGS) -o lambda.kern lambda.o



arch_clean:
	@rm -f common.o lambda.o arch.a lambda.kern


common_msg:
	@echo -e "\033[33m  \033[1mBuilding common\033[0m"

arch_msg:
	@echo -e "\033[33m  \033[1mBuilding x86-specific code\033[0m"
