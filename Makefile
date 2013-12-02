MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

SRCS       = $(wildcard $(SRC)/boot/*.c) $(wildcard $(SRC)/main/*.c) $(wildcard $(SRC)/dev/*.c) $(wildcard $(SRC)/dev/vga/*.c)
ASSRCS     = $(wildcard $(SRC)/boot/*.s) $(wildcard $(SRC)/main/*.s) $(wildcard $(SRC)/dev/*.s) $(wildcard $(SRC)/dev/vga/*.s)
ASMSRCS    = $(wildcard $(SRC)/boot/*.asm) $(wildcard $(SRC)/main/*.asm) $(wildcard $(SRC)/dev/*.asm) $(wildcard $(SRC)/dev/vga/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(COBJS) $(ASMOBJS) $(ASOBJS)

CC         = gcc
AS         = gcc
CFLAGS     = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -O2
ASFLAGS    = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -O2
NASMFLAGS  = -f elf



all:     printinfo link
	
printinfo:
	@echo -e "\033[32mBuilding kernel\033[0m"

link:   $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@ld -melf_i386 -T link.ld -o lambda.kern $(ASOBJS) $(ASMOBJS) $(COBJS)



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
	@rm -f $(OBJS)
	@rm -f lambda.kern