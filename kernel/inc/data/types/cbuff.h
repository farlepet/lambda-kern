#ifndef DATA_TYPES_CBUFF_H
#define DATA_TYPES_CBUFF_H

#include <stdint.h>

/** FIFO cicular buffer */
typedef struct {
    uint32_t begin; /** Index of first written byte */
    uint32_t count; /** Number of readable bytes in the buffer */
    uint32_t size;  /** Size of the buffer */
    uint8_t *buff;  /** The buffer */
} cbuff_t;

/** Helper to create a static cbuff */
#define STATIC_CBUFF(SZ) { .size = (SZ), .buff = (uint8_t[(SZ)]){ 0, }}

#endif // CBUFF_H
