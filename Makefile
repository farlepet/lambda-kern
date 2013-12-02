MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

BOOTSRC    = $(wildcard $(SRC)/boot/*.c)
MAINSRC    = $(wildcard $(SRC)/main/*.c)

ASBOOTSRC = $(wildcard $(SRC)/boot/*.s)
ASMAINSRC = $(wildcard $(SRC)/main/*.s)

ASMBOOTSRC = $(wildcard $(SRC)/boot/*.asm)
ASMMAINSRC = $(wildcard $(SRC)/main/*.asm)

SRCS       = $(BOOTSRC)    $(MAINSRC)
ASSRCS     = $(ASBOOTSRC)  $(ASMAINSRC)
ASMSRCS    = $(ASMBOOTSRC) $(ASMMAINSRC)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

CC         = gcc
AS         = gcc
CFLAGS     = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -O2
ASFLAGS    = -m32 -I$(MAINDIR)/kernel/inc -nostdlib -nostdinc -fno-builtin -Wall -Wextra -Werror -O2
NASMFLAGS  = -f elf



all:     printinfo link
	
printinfo:
	@echo -e "\033[32mBuilding kernel\033[0m"

link:   $(COBJS) $(ASMOBJS) $(ASOBJS)
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
	@rm -f $(SRC)/*/*.o
	@rm -f lambda.kern