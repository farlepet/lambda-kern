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

/* Errors returned by cbuff functions */
#define CBUFF_ERR_FULL   0x70000000 /* Buffer is full */
#define CBUFF_ERR_INVAL  0x60000000 /* Buffer is invalid */
#define CBUFF_ERR_INVLD  0x50000000 /* Invalid data */
#define CBUFF_ERR_EMPTY  0x40000000 /* Buffer is empty */
#define CBUFF_ERR_NENOD  0x30000000 /* Not enough data in buffer to satisfy the request */
#define CBUFF_ERR_MASK   0xF0000000 /* Where error bits are located */

/** Helper to create a static cbuff */
#define STATIC_CBUFF(SZ) { .size = (SZ), .buff = (uint8_t[(SZ)]){ 0, }}

#endif // CBUFF_H
