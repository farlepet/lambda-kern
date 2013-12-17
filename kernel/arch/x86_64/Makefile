KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)


SRCS       = $(wildcard $(SRC)/boot/*.c)   $(wildcard $(SRC)/dev/*.c)        $(wildcard $(SRC)/dev/vga/*.c)      $(wildcard $(SRC)/mm/*.c)   \
             $(wildcard $(SRC)/intr/*.c)   $(wildcard $(SRC)/dev/keyb/*.c)   $(wildcard $(SRC)/dev/serial/*.c)

ASSRCS     = $(wildcard $(SRC)/boot/*.s)   $(wildcard $(SRC)/dev/*.s)        $(wildcard $(SRC)/dev/vga/*.s)      $(wildcard $(SRC)/mm/*.s)   \
             $(wildcard $(SRC)/intr/*.s)   $(wildcard $(SRC)/dev/keyb/*.s)   $(wildcard $(SRC)/dev/serial/*.s)

ASMSRCS    = $(wildcard $(SRC)/boot/*.asm) $(wildcard $(SRC)/dev/*.asm)      $(wildcard $(SRC)/dev/vga/*.asm)    $(wildcard $(SRC)/mm/*.asm) \
             $(wildcard $(SRC)/intr/*.asm) $(wildcard $(SRC)/dev/keyb/*.asm) $(wildcard $(SRC)/dev/serial/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(ASOBJS) $(COBJS) $(ASMOBJS)

CC         = gcc
AS         = gcc
CFLAGS     = -m64 -g -I$(KERNDIR)/inc -I$(MAINDIR) -mcmodel=kernel -mno-red-zone -ffreestanding -nostdlib -nostdinc -Wall -Wextra -Werror -DARCH_X86_64
ASFLAGS    = -m64 -g -I$(KERNDIR)/inc -I$(MAINDIR) -mcmodel=kernel -mno-red-zone -ffreestanding -nostdlib -nostdinc -Wall -Wextra -Werror -DARCH_X86_64
NASMFLAGS  = -felf64 -P../../inc/asm/pusha64.inc

all:     link

link:   $(OBJS)
	@ar rvs --target=elf64-x86-64 arch.a $(OBJS) > /dev/null



%.o: %.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.asm
	@echo -e "\033[32m    \033[1mNASM\033[21m  \033[34m$<\033[0m"
	@nasm $(NASMFLAGS) -o $@ $<



clean:
	@rm -f $(OBJS)
	@rm -f x86_64.arch.a