#include <kern/cmdline.h>
#include <main/main.h>
#include <err/panic.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <string.h>
#include <stdlib.h>

static const char  *_cmdline       = NULL;
static const char **_cmdline_vars  = NULL;
static       size_t _cmdline_nvars = 0;

static const char *_find_var(const char *var) {
    for(size_t i = 0; i < _cmdline_nvars; i++) {
        size_t sz = strlen(_cmdline_vars[i]);
        for(size_t j = 0; j < sz; j++) {
            if(_cmdline_vars[i][j] == '=') {
                sz = j;
                break;
            }
        }
        if((sz == strlen(var)) &&
            !strncmp(var, _cmdline_vars[i], sz)) {
            return _cmdline_vars[i];
        }
    }
    return NULL;
}

void cmdline_set(const char *cmdline) {
    _cmdline = cmdline;
}

void cmdline_init(void) {
    const char *cmdline = _cmdline;

    /* Skip any initial whitespace */
    while(*cmdline == ' ') cmdline++;

    if(strlen(cmdline) == 0) {
        return;
    }
    
    size_t count = 1;

    /* Count number of cmdline options */
    while(*cmdline) {
        /* TODO: Support quoted strings */
        if(*cmdline == ' ') {
            count++;
            while(*cmdline == ' ') {
                cmdline++;
            }
        }
        cmdline++;
    }

    /* NOTE: This could be done in a single alloc to save on allocation overhead,
     * but it's probably not really worth it. */
    _cmdline_vars = (const char **)kmalloc(count * sizeof(char *));
    cmdline = _cmdline;

    /* Copy cmdline options */
    for(size_t i = 0; i < count; i++) {
        const char *str = cmdline;
        size_t j = 0;
        for(; str[j] && str[j] != ' '; j++);

        char *var = (char *)kmalloc(j+1);
        memcpy(var, str, j);
        var[j] = '\0';
        _cmdline_vars[i] = var;

        cmdline = &str[j+1];
        while(*cmdline == ' ') cmdline++;
    }

    _cmdline_nvars = count;

}

static void _handle_option(const char *var, char *opt, size_t len) {
    const char *val;
    if((val = cmdline_getstr(var)) != NULL) {
        if((strlen(val)+1) > len) {
            kpanic("cmdline::_handle_option: Provided [%s] too long!", var);
        }
        strcpy(opt, val);
    }
}

void cmdline_handle_common(void) {
    _handle_option("init", boot_options.init_executable,   sizeof(boot_options.init_executable));
    _handle_option("cpio", boot_options.init_ramdisk_name, sizeof(boot_options.init_ramdisk_name));
    if(cmdline_getbool("kterm")) {
        boot_options.init_executable[0] = '\0';
    }

    const char *dbglvl = cmdline_getstr("debug_level");
    if(dbglvl) {
        while(*dbglvl) {
            debug_source_e src;
            if(*dbglvl == '*') {
                src = DEBUGSRC_MAX;
                dbglvl++;
            } else {
                src = strtoul(dbglvl, (char **)&dbglvl, 10);
            }
            if(*dbglvl != ':') break; /* Bad formatting */
            dbglvl++;

            error_level_e lvl = strtoul(dbglvl, (char **)&dbglvl, 10);
            
            if(src == DEBUGSRC_MAX) {
                for(debug_source_e s = 0; s < DEBUGSRC_MAX; s++) {
                    kdebug_set_errlvl(s, lvl);
                }
            } else {
                kdebug_set_errlvl(src, lvl);
            }

            if(*dbglvl != ',') break; /* Bad formatting, or end of option */
            dbglvl++;
        }
    }
}

const char *cmdline_getstr(const char *var) {
    const char *v = _find_var(var);
    if(v == NULL) {
        return NULL;
    }

    for(size_t i = 0; i < strlen(v); i++) {
        if(v[i] == '=') {
            return &v[i+1];
        }
    }

    return NULL;
}

int cmdline_getbool(const char *var) {
    const char *v = cmdline_getstr(var);

    if((v == NULL) &&
       _find_var(var)) {
        /* Variable exists, but has no '=*' component */
        return 1;
    } else if (v &&
               (!strcmp(v, "true") ||
                !strcmp(v, "TRUE"))) {
        return 1;
    } else {
        return 0;
    }
}
