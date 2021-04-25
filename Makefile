MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

VERBOSE    = 0

# Default architecture
ARCH       = x86

CROSS_COMPILE =
CC            = $(CROSS_COMPILE)gcc
AS            = $(CROSS_COMPILE)gcc
LD            = $(CROSS_COMPILE)ld
AR            = $(CROSS_COMPILE)ar
STRIP         = $(CROSS_COMPILE)strip

export CC
export AS
export LD
export AR
export STRIP
export CROSS_COMPILE
export CFLAGS
export LDFLAGS
export VERBOSE

SRCS       = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*/*.c)
OBJS       = $(patsubst %.c,%.o,$(SRCS))
DEPS       = $(patsubst %.c,%.d,$(SRCS))

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty=\* --always --tags)"

CFLAGS    += -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/$(ARCH)/inc/ \
			 -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -O2 \
			 -pipe -g -fno-stack-protector -fdata-sections -ffunction-sections \
			 -DKERNEL_GIT=\"$(GIT_VERSION)\"

CPIOFILES = $(shell find initrd/)

all: printinfo arch_all

# Architecture-specific makefile options
include kernel/arch/$(ARCH)/arch.mk

ifeq ($(CC), clang)
# TODO: Take the time to go through all these -Wno- commands to fix easy-to-fix errors
CFLAGS += -Weverything -Wno-incompatible-library-redeclaration -Wno-reserved-id-macro -Wno-newline-eof \
		  -Wno-language-extension-token \
		  -Wno-strict-prototypes \
		  -Wno-missing-variable-declarations \
		  -Wno-padded \
		  -Wno-sign-conversion \
		  -Wno-documentation \
		  -Wno-missing-prototypes \
		  -Wno-comma \
		  -Wno-cast-qual \
		  -Wno-pedantic \
		  -Wno-shadow \
		  -Wno-implicit-int-conversion \
		  -Wno-atomic-implicit-seq-cst \
		  -Wno-bad-function-cast \
		  -Wno-cast-align
else
# Temporary(?) fix for syscall function casting in GCC
CFLAGS += -Wno-cast-function-type
endif


printinfo:
	@echo -e "\033[32mBuilding kernel\033[0m"


-include $(DEPS)

%.o: %.c
ifeq ($(VERBOSE), 0)
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
else
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
endif

arch.a: arch_msg
	@cd $(MAINDIR)/kernel/arch/$(ARCH); $(MAKE) CC=$(CC) AS=$(AS)
	@cp $(MAINDIR)/kernel/arch/$(ARCH)/arch.a ./arch.a

symbols.o: lambda.o
	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	@scripts/symbols > symbols.c
	@$(CC) $(CFLAGS) -c -o symbols.o symbols.c

initrd.cpio: $(CPIOFILES)
	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../initrd.cpio &> /dev/null

initrd.o: initrd.cpio
	@echo -e "\033[33m  \033[1mGenerating InitCPIO Object\033[0m"
	@$(LD) $(LDARCH) -r -b binary initrd.cpio -o initrd.o


clean: arch_clean
	@echo -e "\033[33m  \033[1mCleaning sources\033[0m"
	@rm -f $(OBJS) $(DEPS)
	@rm -f initrd.cpio symbols.o symbols.c
	@rm -r -f doc
	@cd $(MAINDIR)/kernel/arch/$(ARCH); make clean

documentation:
	@echo -e "\033[32mGenerating documentation\033[0m"
	@doxygen Doxyfile
	# Remove the following lines of you are generating documentation yourself
	@cp -r doc/html/* ../lambda-os-doc/
	@rm -r doc
	@cd ../lambda-os-doc/lambda-os/; git add --all; git commit -a; git push origin gh-pages

cppcheck:
	@cppcheck --enable=all --suppress=arithOperationsOnVoidPointer -I kernel/inc -I kernel/arch/$(ARCH)/inc kernel/
