#include <mm/mmap.h>

void *mmap(void *addr, size_t len, int prot, int flags, int filedes, off_t off) {
    /* TODO */
    (void)addr;
    (void)len;
    (void)prot;
    (void)flags;
    (void)filedes;
    (void)off;

    return NULL;
}

int munmap(void *addr, size_t len) {
    /* TODO */
    (void)addr;
    (void)len;

    return 0;
}