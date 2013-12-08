KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)


SRCS       = $(wildcard $(SRC)/boot/*.c)   $(wildcard $(SRC)/dev/*.c)        $(wildcard $(SRC)/dev/vga/*.c)      $(wildcard $(SRC)/mm/*.c) \
             $(wildcard $(SRC)/intr/*.c)   $(wildcard $(SRC)/dev/keyb/*.c)   $(wildcard $(SRC)/dev/serial/*.c)

ASSRCS     = $(wildcard $(SRC)/boot/*.s)   $(wildcard $(SRC)/dev/*.s)        $(wildcard $(SRC)/dev/vga/*.s)      $(wildcard $(SRC)/mm/*.s) \
             $(wildcard $(SRC)/intr/*.s)   $(wildcard $(SRC)/dev/keyb/*.s)   $(wildcard $(SRC)/dev/serial/*.s)

ASMSRCS    = $(wildcard $(SRC)/boot/*.asm) $(wildcard $(SRC)/dev/*.asm)      $(wildcard $(SRC)/dev/vga/*.asm)    $(wildcard $(SRC)/mm/*.asm) \
             $(wildcard $(SRC)/intr/*.asm) $(wildcard $(SRC)/dev/keyb/*.asm) $(wildcard $(SRC)/dev/serial/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(ASOBJS) $(COBJS) $(ASMOBJS)

CC         = gcc
AS         = gcc
CFLAGS     = -m32 -I$(KERNDIR)/inc -I$(MAINDIR) -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86
ASFLAGS    = -m32 -I$(KERNDIR)/inc -I$(MAINDIR) -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -DARCH_X86
NASMFLAGS  = -f elf

all:     link

link:   $(OBJS)
	@ar rvs --target=elf32-i386 arch.a $(OBJS) > /dev/null



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
	@rm -f x86.arch.a