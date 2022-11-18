#include <proc/ktask/kinput.h>
#include <lambda/export.h>
#include <proc/ktasks.h>
#include <err/error.h>
#include <err/panic.h>
#include <time/time.h>
#include <fs/stream.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <mm/mm.h>
#include <string.h>
#include <fs/fs.h>
#include <io/output.h>
#include <sys/stat.h>
#include <mod/module.h>

#define prompt  "%skterm\e[0m> ", \
                (retval ? "\e[31m" : "\e[32m")

static int kterm_help(int, char **);
static int kterm_exec(int, char **);
static int kterm_ls(int, char **);
static int kterm_mod(int, char **);
static int kterm_dbgc(int, char **);

struct kterm_entry {
    uint32_t   hash;
    char *string;
    int (*function)(int, char **);
};

static struct kterm_entry kterm_ents[] = {
    { 0, "help",   &kterm_help   },
    { 0, "exec",   &kterm_exec   },
    { 0, "ls",     &kterm_ls     },
    { 0, "mod",    &kterm_mod    },
    { 0, "dbgc",   &kterm_dbgc   },
};

static uint32_t _hash(char *str) {
    uint32_t hash = 5381;
    while (*str)
        hash = ((hash << 5) + hash) + (uint8_t)*str++;

    return hash;
}

#define KTERM_STREAM_LEN 64

static kfile_hand_t *kterm_stdin = NULL;
static kfile_t *_stdin = NULL;

__noreturn void kterm_task() {
    uint32_t i = 0;
    for(; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++)
        kterm_ents[i].hash = _hash(kterm_ents[i].string);

    int retval = 0;
    
    _stdin = stream_create(KTERM_STREAM_LEN);
    if(!_stdin) {
        kpanic("kterm: Could not create main input stream!");
    }

    kterm_stdin = fs_handle_create_open(_stdin, OFLAGS_READ);
    if(!kterm_stdin) {
        kpanic("kterm: Could not create main input stream - kterm side!");
    }
    kinput_dest = fs_handle_create_open(_stdin, OFLAGS_WRITE);
    if(!kinput_dest) {
        kpanic("kterm: Could not create main input stream - kinput side!");
    }
    
    
    kprintf("\n+----------------------------+\n"
            "| Kernel debug shell \"kterm\" |\n"
            "+----------------------------+\n\n");

    for(;;) {
        kprintf(prompt);

        char input[512];
        char *argv[32];
        int iloc = 0;

        memset(input, 0, 512);
        memset(argv,  0, sizeof(char *)*32);

        char t = 0;
        while(1) {
            while(!fs_read(kterm_stdin, 0, 1, (uint8_t *)&t)) {
                delay(10);
            }

            if(t == '\n' || t == '\r') break;
            if(t == '\b') {
                iloc--;
                input[iloc] = ' ';
                kprintf("\b");
                continue;
            }

            if(t != 0) {
                input[iloc++] = t;
            }
        }

        kprintf("\n");

        uint32_t inlen   = strlen(input);
        int cstring = 0;
        int argidx  = 0;
        for(i = 0; i <= inlen; i++) {
            if(input[i] == ' ' || input[i] == '\t' || input[i] == '\0') {
                argv[argidx++] = &input[cstring];
                input[i] = 0;
                cstring = i+1;
            }
        }



        //kprintf("%s\n", input);
        uint32_t h = _hash(argv[0]);
        int fnd = 0;
        for(i = 0; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++) {
            if(kterm_ents[i].hash == h)
                if(!strcmp(kterm_ents[i].string, argv[0])) {
                    retval = kterm_ents[i].function(argidx, argv);
                    fnd = 1;
                    break;
                }
        }
        if(!fnd)
            kprintf("Could not find command: %s\n", argv[0]);
    }
}


static int kterm_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    kprintf("Kterm help:\n");
    kprintf("    help:   display this page\n");
    kprintf("    exec:   load and run an executable\n");
    kprintf("    ls:     list files in current directory\n");
    kprintf("    dbgc:   run a debug command\n");

    return 0;
}



static int kterm_exec(int argc, char **argv) {
    /* TODO: Update to use execve if this is still useful. */
    (void)argc; (void)argv;
#if 0
    if(argc < 2) {
        kprintf("No executable specified\n");
        return 1;
    }

    void  *exec_data = NULL;
    size_t exec_size = 0;
    int    pid       = 0;
    
    if(fs_read_file_by_path(argv[1], NULL, &exec_data, &exec_size, 0)) {
        memset(argv[1], 0, 128);
        kprintf("Could not open %s!\n", argv[1]);
        return 1;
    }
    
    kprintf("%s loaded\n", argv[1]);

    if(*(uint32_t *)exec_data == ELF_IDENT) {
        kprintf("Executable is an ELF executable.\n");
        
        pid = load_elf(exec_data, exec_size);

        if(pid < 0) {
            kerror(ERR_WARN, "Could not load executable");
            return 1;
        }
    } else {
        kprintf("Unsupported executable type.\n");
        kfree(exec_data);
        return 1;
    }

    kerror(ERR_DEBUG, "Task PID: %d", pid);
    
    // TODO: Create helper functions for this:
    struct kproc *child = proc_by_pid(pid);
    if(!child) {
        kerror(ERR_WARN, "kterm: Could not find spawned process!");
        return 1;
    }

#define EXEC_STREAM_LEN 256

    struct kfile *stdin = stream_create(EXEC_STREAM_LEN);
    if(!stdin) {
        kerror(ERR_WARN, "kterm: Could not create STDIN!");
        return 1;
    }

    struct kfile *stdout = stream_create(EXEC_STREAM_LEN);
    if(!stdout) {
        kerror(ERR_WARN, "kterm: Could not create STDOUT!");
        return 1;
    }

    struct kfile *stderr = stream_create(EXEC_STREAM_LEN);
    if(!stderr) {
        kerror(ERR_WARN, "kterm: Could not create STDERR!");
        return 1;
    }
    
    kfile_hand_t *stdout_kterm = fs_handle_create_open(stdout, OFLAGS_READ);
    kfile_hand_t *stderr_kterm = fs_handle_create_open(stderr, OFLAGS_READ);
    kfile_hand_t *stdin_user   = fs_handle_create_open(_stdin, OFLAGS_READ);
    kfile_hand_t *stdout_user  = fs_handle_create_open(stdout, OFLAGS_WRITE);
    kfile_hand_t *stderr_user  = fs_handle_create_open(stderr, OFLAGS_WRITE);
    /* TODO: Check for NULL */

    child->open_files[0] = stdin_user;
    child->open_files[1] = stdout_user;
    child->open_files[2] = stderr_user;

    char buffer[EXEC_STREAM_LEN];

    ssize_t sz;
    while(!(child->type & TYPE_ZOMBIE)) {
        sz = fs_read(stdout_kterm, 0, EXEC_STREAM_LEN, (uint8_t *)&buffer);
        for(ssize_t i = 0; i < sz; i++) {
            kput(buffer[i]);
        }

        sz = fs_read(stderr_kterm, 0, EXEC_STREAM_LEN, (uint8_t *)&buffer);
        for(ssize_t i = 0; i < sz; i++) {
            kput(buffer[i]);
        }
    }

    // Flush remaining output:
    sz = fs_read(stdout_kterm, 0, EXEC_STREAM_LEN, (uint8_t *)&buffer);
    for(ssize_t i = 0; i < sz; i++) {
        kput(buffer[i]);
    }

    sz = fs_read(stderr_kterm, 0, EXEC_STREAM_LEN, (uint8_t *)&buffer);
    for(ssize_t i = 0; i < sz; i++) {
        kput(buffer[i]);
    }

    return 0;
#endif
    return -1;
}


static int kterm_ls(int argc, char **argv) {
    (void)argc;
    (void)argv;

    
    struct kfile *f;
    DIR *dir;
    struct dirent *d;

    if(argc > 1) {
        f = fs_find_file(NULL, argv[1]);
        if(!f) {
            kprintf("Could not find directory: %s\n", argv[1]);
            return 1;
        }
        dir = fs_opendir(f);
    } else  {
        dir = fs_opendir(fs_get_root());
    }

    //kprintf("ls: dir: {%08X, %08X, %08X}\n", dir->dir, dir->current, dir->prev);
    
    while((d = fs_readdir(dir))) {
        kfree(d);

        f = fs_dirfile(dir);

        kprintf("%c%c%c%c%c%c%c%c%c%c%s %02d %05d %s\n",
               ((f->flags & FS_DIR)?'d':'-'),
               ((f->pflags&0400)?'r':'-'), ((f->pflags&0200)?'w':'-'), ((f->pflags&04000)?'s':((f->pflags&0100)?'x':'-')),
               ((f->pflags&040)?'r':'-'), ((f->pflags&020)?'w':'-'), ((f->pflags&02000)?'s':((f->pflags&010)?'x':'-')),
               ((f->pflags&04)?'r':'-'), ((f->pflags&02)?'w':'-'), ((f->pflags&01)?'x':'-'),
               ((f->pflags&01000)?"T":" "),
               f->inode, f->length, f->name);
    }
    
    return 0;
}

static int kterm_mod(int argc, char **argv) {
    if(argc < 2) {
        kprintf("No command specified!");
        return 1;
    }

    if(!strcmp(argv[1], "help")) {
        kprintf("mod help\n"
                "  help:        Displays this help message\n"
                "  info <file>: Display information about the specified module\n"
                "  load <file>: Load the specified module\n"
                "  symlist:     List currently exported symbols\n");
    } else if(!strcmp(argv[1], "info")) {
        if(argc < 3) {
            kprintf("No module specified!\n");
            return 1;
        }

        kfile_t *mod_file = fs_find_file(NULL, argv[2]);
        if(!mod_file) {
            kprintf("Could not find %s\n", argv[2]);
            return 1;
        }
        kfile_hand_t *mod  = fs_handle_create_open(mod_file,  OFLAGS_READ);

        kprintf("Loading module info.\n");
        lambda_mod_head_t *mod_head;
        uintptr_t          mod_base;
        Elf32_Ehdr        *mod_elf;
        if(module_read(mod, &mod_head, &mod_base, &mod_elf)) {
            kprintf("Issue while loading module section.\n");
            return 1;
        }

        kprintf("Module info:\n"
                "  Magic:   %08X\n"
                "  Version: %hu\n"
                "  Kernel:  %hu.%hu.%hu\n",
                mod_head->head_magic,
                mod_head->head_version,
                mod_head->kernel.major,
                mod_head->kernel.minor,
                mod_head->kernel.patch);
        
        kprintf("Metadata:\n"
                "  Identifier:   %s\n"
                "  Name:         %s\n"
                "  Description:  %s\n"
                "  License:      %s\n"
                "  Authors:      ",
                mod_head->metadata.ident       ? (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.ident)       : "N/A",
                mod_head->metadata.name        ? (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.name)        : "N/A",
                mod_head->metadata.description ? (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.description) : "N/A",
                mod_head->metadata.license     ? (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.license)     : "N/A");
                
        if(mod_head->metadata.authors) {
            char **authors = (char **)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.authors);
            size_t i = 0;
            while(authors[i]) {
                kprintf("\n    %s", elf_find_data(mod_elf, (uintptr_t)authors[i]));
                i++;
            }
        } else {
            kprintf("N/A");
        }
        kprintf("\n  Requirements: ");
        if(mod_head->metadata.requirements) {
            char **requirements = (char **)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.requirements);
            size_t i = 0;
            while(requirements[i]) {
                kprintf("\n    %s", elf_find_data(mod_elf, (uintptr_t)requirements[i]));
                i++;
            }
        } else {
            kprintf("N/A");
        }
        kprintf("\n");
    } else if (!strcmp(argv[1], "load")) {
        if(argc < 3) {
            kprintf("No module specified!\n");
            return 1;
        }

        kfile_t *mod_file = fs_find_file(NULL, argv[2]);
        if(!mod_file) {
            kprintf("Could not find %s\n", argv[2]);
            return 1;
        }
        kfile_hand_t *mod  = fs_handle_create_open(mod_file,  OFLAGS_READ);
        
        kprintf("Installing module\n");
        if (module_install(mod)) {
            return -1;
        }
    } else if (!strcmp(argv[1], "symlist")) {
        kprintf("Exported symbols:\n");
        lambda_symbol_t *sym = &__lambda_symbols_begin;
        while(sym < &__lambda_symbols_end) {
            kprintf("  %08X: <%s>\n", sym->addr, sym->name);
            sym++;
        }
    } else {
        kprintf("Unknown mod command!\n");
        return 1;
    }

    return 0;
}

static const uint32_t _const_test = 0x98765432;

static int kterm_dbgc(int argc, char **argv) {
    if(argc < 2) {
        kprintf("No command specified!\n");
        return 1;
    }

    if(!strcmp(argv[1], "help")) {
        kprintf("dbgc help\n"
                "  help:      Displays this help message\n"
                "  divzero:   Force divide by zero exception\n"
                "  pagefault: Force page fault by attempting to write to 0x00000004 and 0xFFFFFFFC\n"
                "  mmprot:    Test that certain memory regions cannot be written to\n");
    } else if (!strcmp(argv[1], "divzero")) {
        kprintf("divzero\n");
        arch_div0();
    } else if (!strcmp(argv[1], "pagefault")) {
        kprintf("pagefault (0x00000004)\n");
        /* GCC 12.1.0 is (incorrectly?) complaining about this line:
         *   array subscript 0 is outside array bounds of 'uint32_t[0]' {aka 'unsigned int[]'} */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        *(uint32_t *)0x00000004 = 0xFFFFFFFF;
#pragma GCC diagnostic pop
        kprintf("pagefault (0xFFFFFFFC)\n");
        *(uint32_t *)0xFFFFFFFC = 0xFFFFFFFF;
    } else if (!strcmp(argv[1], "mmprot")) {
        if(argc < 3) {
            kprintf("mmprot subcommand requires test argument!\n");
            return 1;
        }
        if(!strcmp(argv[2], "1")) {
            /* Write to text */
            uint32_t *ptr = (uint32_t *)kterm_dbgc;
            *ptr = 0x12345678;
            kprintf("mm_check_addr: %d\n", mm_check_addr(ptr));
        } else if(!strcmp(argv[2], "2")) {
            /* Write to rodata */
            uint32_t *ptr = (uint32_t *)&_const_test;
            *ptr = 0x12345678;
            kprintf("mm_check_addr: %d\n", mm_check_addr(ptr));
        } else {
            kprintf("Invalid test!\n");
        }
        return 1;
    } else {
        kprintf("Unknown dbgc command!\n");
        return 1;
    }

    return 0;
}
