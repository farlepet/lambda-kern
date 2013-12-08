MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

# Default architecture
ARCH       = X86

SRCS       = $(wildcard $(SRC)/main/*.c)   $(wildcard $(SRC)/dev/*.c)   $(wildcard $(SRC)/dev/video/*.c)   $(wildcard $(SRC)/dev/keyb/*.c)
ASSRCS     = $(wildcard $(SRC)/main/*.s)   $(wildcard $(SRC)/dev/*.s)   $(wildcard $(SRC)/dev/video/*.s)   $(wildcard $(SRC)/dev/keyb/*.s)
ASMSRCS    = $(wildcard $(SRC)/main/*.asm) $(wildcard $(SRC)/dev/*.asm) $(wildcard $(SRC)/dev/video/*.asm) $(wildcard $(SRC)/dev/keyb/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(COBJS) $(ASMOBJS) $(ASOBJS)

CC         = gcc
AS         = gcc

ifeq ($(ARCH), X86)
CFLAGS     = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86
ASFLAGS    = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86

link:   $(OBJS)
	@echo -e "\033[33m  \033[1mBuilding x86-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/x86; make
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@ld -melf_i386 -T link_x86.ld -o lambda.kern $(ASOBJS) $(ASMOBJS) $(COBJS) kernel/arch/x86/arch.a
	@echo -e "\033[33m  \033[1mCreating ISO\033[0m"
	@cp lambda.kern CD/lambda.kern
	@grub-mkrescue -o lambda-os.iso CD
endif

ifeq ($(ARCH), X86_64)
CFLAGS     = -m64 -I$(MAINDIR)/kernel/inc -mcmodel=kernel -ffreestanding -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86_64
ASFLAGS    = -m64 -I$(MAINDIR)/kernel/inc -mcmodel=kernel -ffreestanding -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86_64

link:   $(OBJS)
	@echo -e "\033[33m  \033[1mBuilding x86_64-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/x86_64; make
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@ld -melf_x86_64 -T link_x86_64.ld -z max-page-size=0x1000 -o lambda.kern $(ASOBJS) $(ASMOBJS) $(COBJS) kernel/arch/x86_64/arch.a
	@echo -e "\033[33m  \033[1mCreating ISO\033[0m"
	@cp lambda.kern CD/lambda.kern
	@grub-mkrescue -o lambda-os.iso CD
endif


all: printinfo link iso
	



printinfo:
	@echo -e "\033[32mBuilding kernel\033[0m"



%.o: %.c
	@echo -e "\033[32m  \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m  \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.asm
	@echo -e "\033[32m  \033[1mNASM\033[21m  \033[34m$<\033[0m"
	@nasm $(NASMFLAGS) -o $@ $<



clean:
	@echo -e "\033[33m  \033[1mCleaning sources\033[0m"
	@rm -f $(OBJS)
	@rm -f lambda.kern
	@rm -r -f doc
	@cd $(MAINDIR)/kernel/arch/x86; make clean
	@cd $(MAINDIR)/kernel/arch/x86_64; make clean

docs:
	@echo -e "\033[32mGenerating documentation\033[0m"
	@doxygen Doxyfile
	# Remove the following lines of you are generating documentation yourself
	@cp -r doc/html/* ../lambda-os-doc/lambda-os/
	@rm -r doc
	@cd ../lambda-os-doc/lambda-os/; git add --all; git commit -a; git push origin gh-pages