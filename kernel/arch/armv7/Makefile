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


all:     link

link:   $(OBJS)
	@$(AR) rvs --target=elf32-littlearm arch.a $(OBJS) &> /dev/null

%.o: %.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(AS) $(ASFLAGS) -c -o $@ $<


clean:
	@rm -f $(OBJS)
	@rm -f arch.a
