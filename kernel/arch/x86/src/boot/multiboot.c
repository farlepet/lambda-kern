#include <err/panic.h>
#include <err/error.h>
#include <fs/initrd.h>
#include <main/main.h>
#include <string.h>
#include <video.h>

#include <arch/boot/multiboot.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/io/serial.h>
#  include <arch/mm/paging.h>
#endif


int _handle_cmdline(const char *cmdline) {
    const char *cmd = cmdline;
    const char *end = cmd + strlen(cmd) + 1;
    char tmp[1000];
    
    while(cmd < end) {
        kerror(ERR_BOOTINFO, "CMD: %08X END: %08X", cmd, end);
        int i = 0;
        if(*cmd == 0) break;
        while(*cmd == ' ') cmd++;
        while(*cmd != ' ' && *cmd != 0) tmp[i++] = *cmd++;
        tmp[i] = 0;
        
        kerror(ERR_BOOTINFO, "  cmd: %s", tmp);

        if (!strcmp(tmp, "-cpio")) {
            /* Load CPIO ramdisk */
            while(*cmd == ' ') cmd++;
            if(*cmd == 0) kpanic("No module name supplied after `-cpio`");
            
            size_t j = 0;
            for(; (cmd[j] && cmd[j] != ' ' && j < (INITRD_MODULE_MAX_LEN - 1)); j++) {
                boot_options.init_ramdisk_name[j] = cmd[j];
            }
            boot_options.init_ramdisk_name[j] = '\0';
            
            while(*cmd && *cmd != ' ') cmd++;
        } else if(!strcmp(tmp, "-init")) {
            /* Specify init executable */
            while(*cmd == ' ') cmd++;
            if(*cmd == 0) kpanic("No executable name supplied after `-init`");
            
            size_t j = 0;
            for(; (cmd[j] && cmd[j] != ' ' && j < (INITEXEC_PATH_MAX_LEN - 1)); j++) {
                boot_options.init_executable[j] = cmd[j];
            }
            boot_options.init_executable[j] = '\0';
            
            while(*cmd && *cmd != ' ') cmd++;
        } else if(!strcmp(tmp, "-kterm")) {
            /* Launch the kterm shell rather than spawning the init executable */
            boot_options.init_executable[0] = '\0';
        }
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86) // Names of serial ports will likely be different on different systems
        else if(!strcmp(tmp, "-s"))     boot_options.output_serial = SERIAL_COM1;
        else if(!strcmp(tmp, "-sCOM1")) boot_options.output_serial = SERIAL_COM1;
        else if(!strcmp(tmp, "-sCOM2")) boot_options.output_serial = SERIAL_COM2;
        else if(!strcmp(tmp, "-sCOM3")) boot_options.output_serial = SERIAL_COM3;
        else if(!strcmp(tmp, "-sCOM4")) boot_options.output_serial = SERIAL_COM4;
#endif
    }

    return 0;
}

int _handle_module(uintptr_t start, uintptr_t end, const char *name) {
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    uint32_t b = ((start - (uint32_t)firstframe) / 0x1000);
    for(; b < ((end - (uint32_t)firstframe) / 0x1000) + 1; b++) {
        set_frame(b, 1); // Make sure that the module is not overwritten
        map_page((b * 0x1000) + firstframe, (b * 0x1000) + firstframe, 3);
    }
#endif

#if (!FEATURE_INITRD_EMBEDDED)
    if(!strcmp(name, (const char *)boot_options.init_ramdisk_name)) {
        initrd_mount(fs_root, start, end - start);
    }
#else
    (void)name;
#endif /* (!FEATURE_INITRD_EMBEDDED) */

    return 0;
}

#if (FEATURE_MULTIBOOT == 1)
void multiboot_check_commandline(const mboot_t *head) {
    if(!(head->flags & MBOOT_CMDLINE)) {
        kerror(ERR_BOOTINFO, "No commandline provided");
        return;
    }
    
    _handle_cmdline((const char *)head->cmdline);
}

void multiboot_check_modules(const mboot_t *head) {
    kerror(ERR_BOOTINFO, "Loading GRUB modules");

    if(!strlen((const char *)boot_options.init_ramdisk_name)) {
        kerror(ERR_BOOTINFO, "  -> No initrd module specified.");
        return;
    }

    if(!(head->flags & MBOOT_MODULES)) {
        kerror(ERR_BOOTINFO, "  -> No modules to load");
        return;
    }

    kerror(ERR_BOOTINFO, "Looking for initrd module [%s]", boot_options.init_ramdisk_name);

    mboot_module_t *mod = (mboot_module_t *)head->mod_addr;
    uint32_t modcnt = head->mod_count;

    for(uint32_t i = 0; i < modcnt; i++) {
        kerror(ERR_BOOTINFO, "  -> MOD[%d/%d]: %s", i+1, modcnt, mod->string);

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
#elif (FEATURE_MULTIBOOT == 2)
const mboot_tag_t *_find_tag(const mboot_t *head, uint32_t type, uint32_t idx) {
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
        (const mboot_tag_cmdline_t *)_find_tag(head, MBOOT_TAGTYPE_CMDLINE, 0);
    if(!cmdline) {
        kerror(ERR_BOOTINFO, "No commandline provided");
        return;
    }

    _handle_cmdline(cmdline->cmdline);
}

void multiboot_check_modules(const mboot_t *head) {
    uint32_t idx = 0;
    const mboot_tag_module_t *mod =
        (const mboot_tag_module_t *)_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    while(mod) {
        kerror(ERR_BOOTINFO, "  -> MOD[%d]: %s", idx, mod->name);
        _handle_module(mod->mod_start, mod->mod_end, mod->name);
        mod = (const mboot_tag_module_t *)_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    }
}

void multiboot_locate_modules(const mboot_t *head, uintptr_t *start, uintptr_t *end) {
    uintptr_t _start = UINTPTR_MAX;
    uintptr_t _end   = 0;

    uint32_t idx = 0;
    const mboot_tag_module_t *mod =
        (const mboot_tag_module_t *)_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    while(mod) {
        if(mod->mod_start < _start) {
            _start = mod->mod_start;
        }
        if(mod->mod_end > _end) {
            _end = mod->mod_end;
        }
        mod = (const mboot_tag_module_t *)_find_tag(head, MBOOT_TAGTYPE_MODULE, idx++);
    }

    *start = _start;
    *end   = _end;
}

size_t multiboot_get_upper_memory(const mboot_t *head) {
    const mboot_tag_basicmem_t *mem =
        (const mboot_tag_basicmem_t *)_find_tag(head, MBOOT_TAGTYPE_BASIC_MEMINFO, 0);
    if(!mem) {
        return 0;
    }

    return mem->size_upper * 1024;
}
#endif
