MAINDIR    = $(CURDIR)
SRC        = $(MAINDIR)/kernel/src

# Default architecture
ARCH       = X86

SRCS       = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*/*.c)

CC         = gcc


OBJS       = $(patsubst %.c,%.o,$(SRCS))

ifeq ($(ARCH), X86)
CFLAGS    += -m32 -I$(MAINDIR)/kernel/inc -I$(MAINDIR) -I$(MAINDIR)/kernel/arch/x86/ \
			 -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -DARCH_X86 -O2 \
			 -pipe -g -fno-stack-protector
LDFLAGS    = -melf_i386 -T link_x86.ld

ifeq ($(CC), clang)
CFLAGS += -Wno-incompatible-library-redeclaration 
endif

link:   $(OBJS) CD/boot/grub/stage2_eltorito
	@echo -e "\033[33m  \033[1mBuilding x86-specific bits\033[0m"
	@cd $(MAINDIR)/kernel/arch/x86; make CC=$(CC)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"

	@ld $(LDFLAGS) -r -o lambda.o $(OBJS) kernel/arch/x86/arch.a

	@echo -e "\033[33m  \033[1mCreating symbol table\033[0m"
	@scripts/symbols > symbols.c
	@$(CC) $(CFLAGS) -c -o symbols.o symbols.c

	@ld $(LDFLAGS) -o lambda.kern lambda.o symbols.o
	@cp lambda.kern CD/lambda.kern

	@echo -e "\033[33m  \033[1mGenerating InitCPIO\033[0m"
	@cd initrd; find . | cpio -o -v -O../CD/initrd.cpio &> /dev/null
	@echo -e "\033[33m  \033[1mCreating ISO\033[0m"

	@genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 \
		-boot-info-table -o lambda-os.iso CD

endif # x86

CD/boot/grub/stage2_eltorito:
	@echo -e "\033[33m	\033[1mDownloading GRUB stage 2 binary\033[0m"
	@curl -o CD/boot/grub/stage2_eltorito https://arabos.googlecode.com/files/stage2_eltorito

all: printinfo link


emu:
	@qemu-system-i386 -cdrom lambda-os.iso -serial stdio -machine pc -no-reboot


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
