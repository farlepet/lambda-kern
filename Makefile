MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

# Default architecture
ARCH       = X86

SRCS       = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*/*.c)
ASSRCS     = $(wildcard $(SRC)/*.s) $(wildcard $(SRC)/*/*.s) $(wildcard $(SRC)/*/*/*.s)
ASMSRCS    = $(wildcard $(SRC)/*.asm) $(wildcard $(SRC)/*/*.asm) $(wildcard $(SRC)/*/*/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(COBJS) $(ASMOBJS) $(ASOBJS)

CC         = gcc
AS         = gcc


ifeq ($(ARCH), X86)
CFLAGS     = -m32 -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/x86/ -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -DARCH_X86
ASFLAGS    = -m32 -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/x86/ -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -DARCH_X86

link:   $(OBJS)
	@echo -e "\033[33m  \033[1mBuilding x86-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/x86; make
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@ld -melf_i386 -T link_x86.ld -o lambda.kern $(ASOBJS) $(ASMOBJS) $(COBJS) kernel/arch/x86/arch.a
	@cp lambda.kern CD/lambda.kern
	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../CD/initrd.cpio >& /dev/null
	@echo -e "\033[33m  \033[1mCreating ISO\033[0m"
	@grub-mkrescue -o lambda-os.iso CD
endif


all: printinfo link
	



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
	@rm -f CD/lambda.kern
	@rm -f CD/initrd.cpio
	@cd $(MAINDIR)/kernel/arch/x86; make clean

docs:
	@echo -e "\033[32mGenerating documentation\033[0m"
	@doxygen Doxyfile
	# Remove the following lines of you are generating documentation yourself
	@cp -r doc/html/* ../lambda-os-doc/lambda-os/
	@rm -r doc
	@cd ../lambda-os-doc/lambda-os/; git add --all; git commit -a; git push origin gh-pages