#include <errno.h>
#include <stdint.h>

#include <crypto/comp/comp.h>
#include <crypto/comp/lzop.h>
#include <types.h>


typedef int (*crypto_comp_decomp_func)(const crypto_comp_handle_t *handle,
                                       const void *const input, size_t input_len,
                                       void **const output, size_t *const output_len);

typedef struct crypto_comp_format_struct {
    uint8_t format;
    crypto_comp_decomp_func decomp;
} crypto_comp_format_t;

static const crypto_comp_format_t _comp_formats[] = {
#ifdef CONFIG_CRYPTO_COMP_LZOP
    { .format = CRYPTO_COMP_FMT_LZOP,
      .decomp = lzop_decompress, },
#endif
};

int crypto_comp_decompress(const crypto_comp_handle_t *handle, const void *const input, size_t input_len, void **const output, size_t *const output_len) {
    for (unsigned i = 0; i < ARRAY_SZ(_comp_formats); i++) {
        if (_comp_formats[i].format == handle->format) {
            if (!_comp_formats[i].decomp) {
                return -EINVAL;
            }

            return _comp_formats[i].decomp(handle, input, input_len, output, output_len);
        }
    }

    return -EINVAL;
}

