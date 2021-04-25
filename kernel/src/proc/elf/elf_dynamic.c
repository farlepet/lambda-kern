#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <string.h>
#include <video.h>

/* Used when interp is `lambda.dyn` Likely to be deprecated when an external
 * dynamic linker is created. */
int elf_dynamic_link_kernel(const Elf32_Ehdr *elf) {
	Elf32_Shdr       *sections     = (Elf32_Shdr *)((uintptr_t)elf + elf->e_shoff);

	for(size_t i = 0; i < elf->e_shnum; i++) {
        if(sections[i].sh_type == SHT_DYNAMIC) {

        }
    }

    return 0;
}

/* Only used for `lambda.shared` symbols. */
__unused
static uintptr_t elf_dynamic_resolve_kernel(const char *sym) {
    (void)sym;
    return 0;
}