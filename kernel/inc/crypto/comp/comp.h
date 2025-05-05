#ifndef CRYPTO_COMP_H
#define CRYPTO_COMP_H

#include <stddef.h>

#include <crypto/comp/types/comp.h>

/**
 * @brief Decompress compressed data
 *
 * @param handle Crypto compression handle, includes the type of compression
 * @param input Input buffer
 * @param input_len Size of input buffer
 * @param output Where to store pointer to allocated output buffer
 * @param output_len Where to store decompressed data length
 *
 * @return 0 on success, else negative error code
 */
int crypto_comp_decompress(const crypto_comp_handle_t *handle, const void *const input, size_t input_len, void **const output, size_t *const output_len);

#endif

