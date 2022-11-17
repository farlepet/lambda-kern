MAINDIR    = .
KERNEL     = $(MAINDIR)/kernel
BUILDDIR   = $(MAINDIR)/build/$(ARCH)/$(CPU)/$(HW)

VERBOSE     = 0
EMBEDINITRD = 0

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

# Default architecture
ARCH       = x86

ifeq ($(CC),)
	CC      := $(CROSS_COMPILE)gcc
endif
ifeq ($(AS),)
	AS      := $(CROSS_COMPILE)gcc
endif
ifeq ($(LD),)
	LD      := $(CROSS_COMPILE)ld
endif
ifeq ($(AR),)
	AR      := $(CROSS_COMPILE)ar
endif
ifeq ($(STRIP),)
	STRIP   := $(CROSS_COMPILE)strip
endif
ifeq ($(OBJCOPY),)
	OBJCOPY := $(CROSS_COMPILE)objcopy
endif


GIT_VERSION := "$(shell git describe --abbrev=8 --dirty=\* --always --tags)"

CFLAGS    += -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/$(ARCH)/inc/ \
			 -nostdinc -ffreestanding -Wall -Wextra -Werror -O2 \
			 -pipe -g -fdata-sections -ffunction-sections \
			 -include "config.h" \
			 -DKERNEL_GIT=\"$(GIT_VERSION)\"

KERNSRC    = $(KERNEL)/src
ARCHSRC    = $(MAINDIR)/kernel/arch/$(ARCH)/src
ARCHINC    = $(MAINDIR)/kernel/arch/$(ARCH)/inc


.PHONY: clean documentation cppcheck

.DEFAULT_GOAL=$(BUILDDIR)/lambda.kern

# TODO: Allow selecting of specific source files in a smart way
SRCS       = $(wildcard $(KERNSRC)/*.c) $(wildcard $(KERNSRC)/*/*.c) $(wildcard $(KERNSRC)/*/*/*.c)

OBJS       = $(filter %.o,$(patsubst $(KERNEL)/%.c,$(BUILDDIR)/%.o,$(SRCS)) \
                          $(patsubst $(KERNEL)/%.s,$(BUILDDIR)/%.o,$(SRCS)))
DEPS       = $(filter %.d,$(patsubst $(KERNEL)/%.c,$(BUILDDIR)/%.d,$(SRCS)))

# Architecture-specific makefile options
include kernel/arch/$(ARCH)/arch.mk


ifeq ($(CC), clang)
CFLAGS += -Weverything                \
		  -Wno-reserved-id-macro       \
		  -Wno-newline-eof             \
		  -Wno-padded                  \
		  -Wno-sign-conversion         \
		  -Wno-documentation           \
		  -Wno-cast-qual               \
		  -Wno-pedantic                \
		  -Wno-implicit-int-conversion \
		  -Wno-atomic-implicit-seq-cst \
		  -Wno-bad-function-cast       \
		  -Wno-cast-align              \
		  -Wno-packed                  \
		  -Wno-unknown-warning-option  \
		  -Wno-date-time               \
		  -Wno-reserved-identifier     \
		  -Wno-extra-semi-stmt
endif


$(BUILDDIR)/symbols.o: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	$(Q) scripts/symbols > $(BUILDDIR)/symbols.c
	$(Q) $(CC) $(CFLAGS) -c -o $(BUILDDIR)/symbols.o $(BUILDDIR)/symbols.c

# TODO: Only include this if FEATURE_INITRD_EMBEDDED
$(BUILDDIR)/initrd.o: initrd.cpio
	@echo -e "\033[33m  \033[1mGenerating embedded InitRD object\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(LD) $(LDARCH) -r -b binary $< -o $@


clean:
	@echo -e "\033[33m  \033[1mCleaning sources\033[0m"
	$(Q) rm -rf $(BUILDDIR)
	$(Q) rm -rf doc

documentation:
	@echo -e "\033[32mGenerating documentation\033[0m"
	@doxygen Doxyfile
	# Remove the following lines of you are generating documentation yourself
	@cp -r doc/html/* ../lambda-os-doc/
	@rm -r doc
	@cd ../lambda-os-doc/lambda-os/; git add --all; git commit -a; git push origin gh-pages

cppcheck:
	$(Q) cppcheck --enable=all --suppress=arithOperationsOnVoidPointer --suppress=unusedFunction -I kernel/inc -I kernel/arch/$(ARCH)/inc kernel/

scan-build:
	@scan-build --use-cc=$(CC) -analyze-headers $(MAKE)


$(BUILDDIR)/%.o: $(KERNEL)/%.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.o: $(KERNEL)/%.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(AS) $(ASFLAGS) -c -o $@ $<

-include $(DEPS)
