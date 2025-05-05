#ifndef CRYPTO_COMP_TYPES_COMP_H
#define CRYPTO_COMP_TYPES_COMP_H

#include <stdint.h>

typedef enum crypto_comp_format_enum {
    CRYPTO_COMP_FMT_NONE = 0,
#ifdef CONFIG_CRYPTO_COMP_LZOP
    CRYPTO_COMP_FMT_LZOP,
#endif
} crypto_comp_format_e;

typedef struct crypto_comp_handle_struct {
    /** Compression format */
    uint8_t format;
    /* Per-format options to go into a union here */
} crypto_comp_handle_t;

#endif

