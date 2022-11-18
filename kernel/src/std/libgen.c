#include <libgen.h>
#include <stddef.h>

#include <lambda/export.h>

char *dirname(char *path) {
    char *p1 = path, *p2 = NULL;
    while(*p1) {
        if(*p1 == '/') {
            p2 = p1;
        }
        p1++;
    }

    if(p2 == NULL) {
        path[0] = '.';
        path[1] = '\0';
    } else {
        p2[0] = '\0';
    }
    return path;
}
EXPORT_FUNC(dirname);

char *basename(char *path) {
    char *p1 = path, *p2 = NULL;

    while(*p1) {
        if(*p1 == '/') {
            p2 = p1;
        }
        p1++;
    }

    if(p2 == NULL) {
        return path;
    } else {
        return &p2[1];
    }
}
EXPORT_FUNC(basename);
