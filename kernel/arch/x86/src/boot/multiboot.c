#include <kern/cmdline.h>
#include <err/panic.h>
#include <err/error.h>
#include <fs/initrd.h>
#include <main/main.h>
#include <mm/mmu.h>
#include <string.h>
#include <io/output.h>

#include <arch/boot/multiboot.h>
#include <arch/mm/paging.h>


static int _handle_module(uintptr_t start, uintptr_t end, const char *name) {
    mmu_map(start, start, (end - start), MMU_FLAG_READ);

#ifndef CONFIG_EMBEDDED_INITRD
    if(!strcmp(name, (const char *)boot_options.init_ramdisk_name)) {
        initrd_mount(fs_get_root(), start, end - start);
    }
#else
    (void)name;
#endif

    return 0;
}

#if (CONFIG_MULTIBOOT_VERSION == 1)
void multiboot_check_commandline(const mboot_t *head) {
    if(!(head->flags & MBOOT_CMDLINE)) {
        kerror(ERR_INFO, "No commandline provided");
        return;
    }

    cmdline_set((const char *)head->cmdline);
}

void multiboot_check_modules(const mboot_t *head) {
    kerror(ERR_INFO, "Loading GRUB modules");

    if(!strlen((const char *)boot_options.init_ramdisk_name)) {
        kerror(ERR_INFO, "  -> No initrd module specified.");
        return;
    }

    if(!(head->flags & MBOOT_MODULES)) {
        kerror(ERR_INFO, "  -> No modules to load");
        return;
    }

    kerror(ERR_DEBUG, "Looking for initrd module [%s]", boot_options.init_ramdisk_name);

    mboot_module_t *mod = (mboot_module_t *)head->mod_addr;
    uint32_t modcnt = head->mod_count;

    for(uint32_t i = 0; i < modcnt; i++) {
        kerror(ERR_DEBUG, "  -> MOD[%d/%d]: %s", i+1, modcnt, mod->string);

        _handle_module(mod->mod_start, mod->mod_end, (const char *)mod->string);

        mod++;
    }
}

void multiboot_locate_modules(const mboot_t *head, uintptr_t *start, uintptr_t *end) {
    uintptr_t _start = UINTPTR_MAX;
    uintptr_t _end   = 0;

    if(!(head->flags & MBOOT_MODULES)) {
        *start = _start;
        *end   = _end;
        return;
    }

    mboot_module_t *mod = (mboot_module_t *)head->mod_addr;
    uint32_t modcnt = head->mod_count;

    for(uint32_t i = 0; i < modcnt; i++) {
        if(mod->mod_start < _start) {
            _start = mod->mod_start;
        }
        if(mod->mod_end > _end) {
            _end = mod->mod_end;
        }

        mod++;
    }

    *start = _start;
    *end   = _end;
}

size_t multiboot_get_upper_memory(const mboot_t *head) {
    if(!(head->flags & MBOOT_MEMINFO)) {
        return 0;
    }

    return head->mem_upper * 1024;
}
#elif (CONFIG_MULTIBOOT_VERSION == 2)
const mboot_tag_t *multiboot_find_tag(const mboot_t *head, uint32_t type, uint32_t idx) {
    const mboot_tag_t *tag = (const mboot_tag_t *)&head->tags;
    uintptr_t end = ((uintptr_t)head->tags + head->size);
    uint32_t _idx = 0;

    while((uintptr_t)tag < end) {
        if(tag->type == type) {
            if(_idx == idx) {
                return tag;
            }
            _idx++;
        } else if(tag->type == 0) {
            /* End of list */
            break;
        }
        size_t sz = tag->size;
        if(sz & 0x7) {
            sz = (sz | 0x07) + 1;
        }
        tag = (mboot_tag_t *)((uintptr_t)tag + sz);
    }
    return NULL;
}

void multiboot_check_commandline(const mboot_t *head) {
    const mboot_tag_cmdline_t *cmdline =
        (const mboot_tag_cmdline_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_CMDLINE, 0);
    if(!cmdline) {
        kerror(ERR_INFO, "No commandline provided");
        return;
    }

    cmdline_set(cmdline->cmdline);
}

void multiboot_check_modules(const mboot_t *head) {
    uint32_t idx = 0;
    const mboot_tag_module_t *mod =
        (const mboot_tag_module_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    while(mod) {
        kerror(ERR_DEBUG, "  -> MOD[%d]: %s", idx, mod->name);
        _handle_module(mod->mod_start, mod->mod_end, mod->name);
        mod = (const mboot_tag_module_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    }
}

void multiboot_locate_modules(const mboot_t *head, uintptr_t *start, uintptr_t *end) {
    uintptr_t _start = UINTPTR_MAX;
    uintptr_t _end   = 0;

    uint32_t idx = 0;
    const mboot_tag_module_t *mod =
        (const mboot_tag_module_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    while(mod) {
        if(mod->mod_start < _start) {
            _start = mod->mod_start;
        }
        if(mod->mod_end > _end) {
            _end = mod->mod_end;
        }
        mod = (const mboot_tag_module_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    }

    *start = _start;
    *end   = _end;
}

size_t multiboot_get_upper_memory(const mboot_t *head) {
    const mboot_tag_basicmem_t *mem =
        (const mboot_tag_basicmem_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_BASIC_MEMINFO, 0);
    if(!mem) {
        return 0;
    }

    return mem->size_upper * 1024;
}
#endif
