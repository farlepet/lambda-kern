KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)


SRCS       = $(wildcard $(SRC)/*.c)   $(wildcard $(SRC)/*/*.c)   $(wildcard $(SRC)/*/*/*.c)
ASSRCS     = $(wildcard $(SRC)/*.s)   $(wildcard $(SRC)/*/*.s)   $(wildcard $(SRC)/*/*/*.s)
ASMSRCS    = $(wildcard $(SRC)/*.asm) $(wildcard $(SRC)/*/*.asm) $(wildcard $(SRC)/*/*/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(ASOBJS) $(COBJS) $(ASMOBJS)

CC         = clang
AS         = clang
CFLAGS     = -m32 -I$(KERNDIR)/inc -I$(MAINDIR) -nostdlib -nostdinc -Wall -Wextra -Werror \
				 -DARCH_X86 -O2 -fdiagnostics-show-category=name
ASFLAGS    = -m32
NASMFLAGS  = -f elf

all:     link

link:   $(OBJS)
	@ar rvs --target=elf32-i386 arch.a $(OBJS) &> /dev/null


%.o: %.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(AS) $(ASFLAGS) -c -o $@ $<

%.o: %.asm
	@echo -e "\033[32m    \033[1mNASM\033[21m  \033[34m$<\033[0m"
	@nasm $(NASMFLAGS) -o $@ $<



clean:
	@rm -f $(OBJS)
	@rm -f arch.a