#ifndef CBUFF_H
#define CBUFF_H

#include <types.h>

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

/**
 * @brief Place byte into circular buffer
 *
 * @param data Byte to place in buffer
 * @param buff Circular buffer in which to place byte
 * @return int 0 on success,
 *             CBUFF_ERR_* on failure
 */
int cbuff_put(uint8_t data, cbuff_t *buff);

/**
 * @brief Read byte from circular buffer
 *
 * @param buff Circular buffer from which to read
 * @return int Read byte on success,
 *             CBUFF_ERR_* on failure
 */
int cbuff_get(cbuff_t *buff);

/**
 * @brief Write data into circular buffer
 *
 * Will return CBUFF_FULL if the entire set of data cannot be placed in the buffer
 *
 * @param data Pointer to data to write into buffer
 * @param size Size of data in bytes
 * @param buff Buffer to write data into
 * @return int 0 on success,
 *             CBUFF_ERR_* on failure
 */
int cbuff_write(const uint8_t *data, size_t size, cbuff_t *buff);

/**
 * @brief Read data from circular buffer
 *
 * @param data Pointer to where to store data
 * @param size Number of bytes to read
 * @param buff Buffer to read from
 * @return int 0 on success,
 *             CBUFF_ERR_* on failure
 */
int cbuff_read(uint8_t *data, size_t size, cbuff_t *buff);


#endif // CBUFF_H