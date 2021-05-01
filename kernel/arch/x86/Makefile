KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/src

SRCS       = $(wildcard $(SRC)/*.c)   $(wildcard $(SRC)/*/*.c)   $(wildcard $(SRC)/*/*/*.c)
ASSRCS     = $(wildcard $(SRC)/*.s)   $(wildcard $(SRC)/*/*.s)   $(wildcard $(SRC)/*/*/*.s)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))

OBJS       = $(ASOBJS) $(COBJS) $(ASMOBJS)
DEPS       = $(patsubst %.o,%.d,$(OBJS))


all:     link

link:   $(OBJS)
	@ar rvs --target=elf32-i386 arch.a $(OBJS) &> /dev/null

-include $(DEPS)

%.o: %.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(AS) $(ASFLAGS) -c -o $@ $<


clean:
	@rm -f $(OBJS) $(DEPS)
	@rm -f arch.a
