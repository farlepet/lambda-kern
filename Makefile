MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src


# Default architecture
ARCH       = x86

CROSS_COMPILE =
CC            = $(CROSS_COMPILE)gcc
AS            = $(CROSS_COMPILE)gcc
LD            = $(CROSS_COMPILE)ld

export CC
export AS
export LD
export CROSS_COMPILE
export CFLAGS
export LDFLAGS

SRCS       = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*/*.c)
OBJS       = $(patsubst %.c,%.o,$(SRCS))

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty=\* --always --tags)"

CFLAGS    += -DKERNEL_GIT=\"$(GIT_VERSION)\"

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


all: printinfo link


printinfo:
	@echo -e "\033[32mBuilding kernel\033[0m"


# gcc:
%.o: %.c
	@echo -e "\033[32m  \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<



clean:
	@echo -e "\033[33m  \033[1mCleaning sources\033[0m"
	@rm -f $(OBJS)
	@rm -f lambda.kern
	@rm -r -f doc
	@rm -f CD/lambda.kern
	@rm -f CD/initrd.cpio
	@rm -f kern.*
	@cd $(MAINDIR)/kernel/arch/x86; make clean

documentation:
	@echo -e "\033[32mGenerating documentation\033[0m"
	@doxygen Doxyfile
	# Remove the following lines of you are generating documentation yourself
	@cp -r doc/html/* ../lambda-os-doc/
	@rm -r doc
	@cd ../lambda-os-doc/lambda-os/; git add --all; git commit -a; git push origin gh-pages

cppcheck:
	@cppcheck --enable=all --suppress=arithOperationsOnVoidPointer -I kernel/inc -I kernel/arch/$(ARCH)/inc kernel/
