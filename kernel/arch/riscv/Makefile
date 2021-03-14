KERNDIR    = $(CURDIR)/../..
MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/src

SRCS       = $(wildcard $(SRC)/*.c)   $(wildcard $(SRC)/*/*.c)   $(wildcard $(SRC)/*/*/*.c)
ASSRCS     = $(wildcard $(SRC)/*.s)   $(wildcard $(SRC)/*/*.s)   $(wildcard $(SRC)/*/*/*.s)

COBJS      = $(patsubst %.c,%.o,$(SRCS))
ASOBJS     = $(patsubst %.s,%.o,$(ASSRCS))

OBJS       = $(ASOBJS) $(COBJS)
DEPS       = $(patsubst %.o,%.d,$(OBJS))


all:     link

link:   $(OBJS)
	@$(AR) rvs --target=elf64-little arch.a $(OBJS) &> /dev/null

-include $(DEPS)

%.o: %.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

%.o: %.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	@$(AS) $(ASFLAGS) -c -o $@ $<


clean:
	@rm -f $(OBJS)
	@rm -f arch.a
