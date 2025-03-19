#ifndef DATA_CBUFF_H
#define DATA_CBUFF_H

#include <stddef.h>

#include <data/types/cbuff.h>

int cbuff_allocate(cbuff_t *buff, size_t size);

void cbuff_free(cbuff_t *buff);

/**
 * @brief Place byte into circular buffer
 *
 * @param data Byte to place in buffer
 * @param buff Circular buffer in which to place byte
 * @return int 0 on success,
 *             -E* on failure
 */
int cbuff_put(uint8_t data, cbuff_t *buff);

/**
 * @brief Read byte from circular buffer
 *
 * @param buff Circular buffer from which to read
 * @return int Read byte on success,
 *             -E* on failure
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
 *
 * @return int 0 on success,
 *             -E* on failure
 */
int cbuff_write(const uint8_t *data, size_t size, cbuff_t *buff);

/**
 * @brief Read data from circular buffer
 *
 * @param data Pointer to where to store data
 * @param size Number of bytes to read
 * @param buff Buffer to read from
 *
 * @return int 0 on success,
 *             -E* on failure
 */
int cbuff_read(uint8_t *data, size_t size, cbuff_t *buff);


#endif // CBUFF_H
