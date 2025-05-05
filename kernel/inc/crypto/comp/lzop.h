#ifndef CRYPTO_COMP_LZOP_H
#define CRYPTO_COMP_LZOP_H

#include <crypto/comp/types/comp.h>

/**
 * @brief Decompress lzop-compressed data using LZO1X
 *
 * @note This should not be used outside of crypto_comp - external users should
 * utilise crypto_comp_decompress instead.
 *
 * @param handle Crypto compression handle
 * @param input Input buffer
 * @param input_len Size of input buffer
 * @param output Where to store pointer to allocated output buffer
 * @param output_len Where to store decompressed data length
 *
 * @return 0 on success, else negative error code
 */
int lzop_decompress(const crypto_comp_handle_t *handle, const void *const input, size_t input_len, void **const output, size_t *const output_len);

#endif

