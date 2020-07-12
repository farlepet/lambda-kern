KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/src

SRCS       = $(wildcard $(SRC)/*.c)   $(wildcard $(SRC)/*/*.c)   $(wildcard $(SRC)/*/*/*.c)
ASSRCS     = $(wildcard $(SRC)/*.s)   $(wildcard $(SRC)/*/*.s)   $(wildcard $(SRC)/*/*/*.s)
ASMSRCS    = $(wildcard $(SRC)/*.asm) $(wildcard $(SRC)/*/*.asm) $(wildcard $(SRC)/*/*/*.asm)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))
ASMOBJS    = $(patsubst %.asm,%.o,$(ASMSRCS))

OBJS       = $(ASOBJS) $(COBJS) $(ASMOBJS)

CC         = gcc
AS         = gcc
CFLAGS     = -m32 -I$(KERNDIR)/inc -I$(MAINDIR)/inc -nostdlib -nostdinc -Wall -Wextra -Werror \
			 -DARCH_X86 -O2 -pipe -g -fno-stack-protector
ASFLAGS    = -m32
NASMFLAGS  = -f elf

CFLAGS    += -march=i586

ifeq ($(CC), clang)
CFLAGS += -Weverything -Wno-incompatible-library-redeclaration -Wno-reserved-id-macro -Wno-newline-eof \
-Wno-strict-prototypes \
-Wno-language-extension-token \
-Wno-padded \
-Wno-missing-variable-declarations \
-Wno-flexible-array-extensions \
-Wno-sign-conversion \
-Wno-documentation \
-Wno-pedantic \
-Wno-missing-prototypes \
-Wno-shadow \
-Wno-bad-function-cast \
-Wno-implicit-int-conversion \
-Wno-atomic-implicit-seq-cst
endif

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
