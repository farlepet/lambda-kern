
include .config

MAINDIR    = .
KERNEL     = $(MAINDIR)/kernel

VERBOSE     = 0

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty=\* --always --tags)"

# Make sure these variables are empty and setup to be immediate
# Output object file list
obj-y     :=
# Assembler flags
asflags-y :=
# C compiler flags
cflags-y  :=
# Linker flags
ldflags-y :=

# Shared library linking step flags
ldsflags-y :=
# Relocatable object linking step flags
ldoflags-y :=
# Final kernel output linking step flags
ldkflags-y :=

ifeq ($(CONFIG_BUILD_USE_CLANG),y)
    CC := clang
    AS := clang

    ifeq ($(CONFIG_BUILD_USE_WEVERYTHING),y)
        cflags-y += -Weverything                 \
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
else
    ifneq ($(CROSS_COMPILE),)
        CC      := $(CROSS_COMPILE)gcc
        AS      := $(CROSS_COMPILE)gcc
        LD      := $(CROSS_COMPILE)ld
        AR      := $(CROSS_COMPILE)ar
        STRIP   := $(CROSS_COMPILE)strip
        OBJCOPY := $(CROSS_COMPILE)objcopy
    endif
endif

cflags-$(CONFIG_BUILD_USE_WERROR) += -Werror

include kernel/module.mk

ifneq ($(CONFIG_EMBEDDED_INITRD_LZOP),y)
    obj-$(CONFIG_EMBEDDED_INITRD) += initrd.o
endif
obj-$(CONFIG_EMBEDDED_INITRD_LZOP) += initrd.lzo.o

BUILDDIR   = $(MAINDIR)/build/$(ARCH)/$(CPU)/$(HW)

OBJS := $(filter %.o,$(patsubst %.o,$(BUILDDIR)/%.o,$(obj-y)))
DEPS := $(filter %.d,$(patsubst %.o,%.d,$(OBJS)))

cflags-y += -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/$(ARCH)/inc/ \
            -ffreestanding -Wall -Wextra -O2 \
            -pipe -g -fdata-sections -ffunction-sections \
            -DKERNEL_GIT=\"$(GIT_VERSION)\"


.PHONY: clean documentation cppcheck

.DEFAULT_GOAL=$(BUILDDIR)/lambda.kern


$(BUILDDIR)/symbols.o: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	$(Q) scripts/symbols > $(BUILDDIR)/symbols.c
	$(Q) $(CC) $(cflags-y) -c -o $(BUILDDIR)/symbols.o $(BUILDDIR)/symbols.c

# TODO: Only include this if FEATURE_INITRD_EMBEDDED
$(BUILDDIR)/initrd.o: initrd.cpio
	@echo -e "\033[33m  \033[1mGenerating embedded InitRD object\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(LD) $(LDARCH) -r -b binary $< -o $@

$(BUILDDIR)/initrd.lzo.o: initrd.cpio.lzo
	@echo -e "\033[33m  \033[1mGenerating embedded compressed InitRD object\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(LD) $(LDARCH) -r -b binary $< -o $@

$(BUILDDIR)/lambda.o: $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	$(Q) $(LD) $(ldoflags-y) -r -o $@ $(OBJS)

$(BUILDDIR)/lambda.shared: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mLinking kernel\033[0m"
	$(Q) $(CC) $(ldsflags-y) -shared -o $@ $< -T $(HWDIR)/hw.ld

$(BUILDDIR)/lambda.kern: $(BUILDDIR)/lambda.o
	@echo -e "\033[33m  \033[1mProducing kernel executable\033[0m"
	$(Q) $(CC) $(ldkflags-y) -o $@ $< -T $(HWDIR)/hw.ld -nostdlib -lgcc


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


$(BUILDDIR)/%.o: %.c .config
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(cflags-y) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.o: %.s .config
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(AS) $(asflags-y) -c -o $@ $<

.config: | .defconfig
	@echo -e "\033[32m\033[1mCopying default .config\033[0m"
	$(Q) cp $| $@

-include $(DEPS)
