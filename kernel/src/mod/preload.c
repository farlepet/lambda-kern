#include <errno.h>
#include <string.h>

#include <mod/module.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <mm/mmap.h>
#include <fs/fs.h>
#include <io/output.h>

int modules_preload(const char *path) {
    void  *pre_data = NULL;
    size_t pre_sz   = 0;
    if(fs_read_file_by_path(path, NULL, &pre_data, &pre_sz, 0)) {
        kdebug(DEBUGSRC_MODULE, ERR_WARN, "Could not read modules preload file `%s`", path);
        return -ENOENT;
    }

    char *text     = pre_data;
#define MOD_MAXPATH 256
    char *mod_path = kmalloc(MOD_MAXPATH+1);
    if(mod_path == NULL) {
        kpanic("Could not allocate memory for module paths!");
    }

    size_t idx = 0;
    while(idx < pre_sz) {
        size_t end = idx;
        while((end < pre_sz) && (text[end] != '\n')) {
            end++;
        }
        
        size_t pathsz = end - idx;
        memcpy(mod_path, &text[idx], pathsz);
        mod_path[pathsz] = 0;

        kdebug(DEBUGSRC_MODULE, ERR_INFO, "Attempting to load module `%s`", mod_path);

        kfile_t *mod_file = fs_find_file(NULL, mod_path);
        if(mod_file == NULL) {
            kdebug(DEBUGSRC_MODULE, ERR_ERROR, "Could not access module `%s`", mod_path);
            kfree(mod_path);
            return -ENOENT;
        }
        kfile_hand_t *mod_hand = fs_handle_create_open(mod_file, OFLAGS_READ);

        if(module_install(mod_hand)) {
            kdebug(DEBUGSRC_MODULE, ERR_ERROR, "Could not load module `%s`", mod_path);
            fs_handle_destroy(mod_hand);
            kfree(mod_path);
            return -EUNSPEC;
        }

        fs_handle_destroy(mod_hand);

        idx = end+1; /* Skip \n */
    }

    kfree(mod_path);

    return 0;
}
