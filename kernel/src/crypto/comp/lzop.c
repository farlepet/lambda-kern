#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <crypto/comp/types/comp.h>
#include <crypto/comp/lzop.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <types.h>

static int _lzo1x_decompress_block(const void *input, size_t input_len, void *output, size_t *output_len);

#define BE16(P) (((uint16_t)((uint8_t *)(P))[0] << 8) | \
                  (uint16_t)((uint8_t *)(P))[1])
#define BE32(P) (((uint16_t)((uint8_t *)(P))[0] << 24) | \
                 ((uint16_t)((uint8_t *)(P))[1] << 16) | \
                 ((uint16_t)((uint8_t *)(P))[2] << 8)  | \
                  (uint16_t)((uint8_t *)(P))[3])
/* Assuming little-endian unaligned-happy system for now */
#define LE16(P) (*(uint16_t *)(P))

static const uint8_t lzop_magic[] = {
    0x89, 'L', 'Z', 'O', 0x00, 0x0d, 0x0a, 0x1a, 0x0a
};

/*
 * LZOP format:
 *   9 bytes: Magic number
 *   2 bytes: Version (big-endian)
 *   2 bytes: Library version
 *   2 bytes: Minimum version required for extraction
 *   1 byte:  Method (LZO type)
 *   If version >= 0x940
 *     1 byte: Level
 *   4 bytes: Flags
 *   If Flags & HAS_FILTER
 *     4 bytes: Filter info
 *   4 bytes: Mode
 *   8 bytes: mtime
 *   1 byte:  Filename length
 *   x bytes: Filename
 *   4 bytes: Header checksum (CRC-32 if Flags & USE_CRC32, else Adler-32)
 *   If Flags & HAS_EXTRA_FIELD
 *     4 bytes: Extra field length
 *     x bytes: Extra field data
 *     4 bytes: Extra field checksum (same algorithm as header checksum)
 *   Repeated:
 *     4 bytes: Uncompressed block size
 *     4 bytes: Compressed block size
 *     4 bytes: Uncompressed block checksum (optional)
 *     4 bytes: Compressed block checksum (optional)
 */

#define LZOP_FLAGS_UNCOMP_USE_ADLER32 (1U <<  0) /**< Adler-32 checksum of uncompressed data included */
#define LZOP_FLAGS_COMP_USE_ADLER32   (1U <<  1) /**< Adler-32 checksum of compressed data included */
#define LZOP_FLAGS_HAS_EXTRA_FIELD    (1U <<  6) /**< Header includes extra field */
#define LZOP_FLAGS_UNCOMP_USE_CRC32   (1U <<  8) /**< CRC-32 checksum of uncompressed data included */
#define LZOP_FLAGS_COMP_USE_CRC32     (1U <<  9) /**< CRC-32 checksum of compressed data included */
#define LZOP_FLAGS_HAS_FILTER         (1U << 11) /**< Filter fields are included */
#define LZOP_FLAGS_USE_CRC32          (1U << 12) /**< Use CRC-32 for header */

int lzop_decompress(const crypto_comp_handle_t *handle, const void *const input, size_t input_len, void **const output, size_t *const output_len) {
    (void)handle;

    if (input_len < sizeof(lzop_magic)) {
        return -EUNSPEC;
    }

    /* TODO: Check for input overruns */

    const uint8_t *inb = input;
    if (memcmp(inb, lzop_magic, sizeof(lzop_magic))) {
        return -EUNSPEC;
    }
    inb += sizeof(lzop_magic);

    uint16_t version = BE16(inb);
    /* Skip: Library version, Minimum version, Method */
    inb += 7;
    if (version >= 0x0940) {
        /* Skip: Level */
        inb++;
    }

    uint32_t flags = BE32(inb);
    inb += 4;
    if (flags & LZOP_FLAGS_HAS_FILTER) {
        /* Skip: Filter */
        inb += 4;
    }

    /* Skip: Mode, mtime */
    inb += 12;

    /* Skip filename */
    uint8_t filename_len = *inb;
    inb += 1 + filename_len;

    /* Skip checksum (TODO: Check it) */
    inb += 4;

    /* Skip extra field */
    if (flags & LZOP_FLAGS_HAS_EXTRA_FIELD) {
        inb += BE32(inb) + 8;
    }

    /* Get output size */
    {
        *output_len = 0;
        const uint8_t *ptr = inb;
        while (ptr < ((uint8_t *)input + input_len)) {
            uint32_t sz = BE32(ptr);
            if (sz == 0) {
                break;
            }
            *output_len += sz;

            ptr += 4;
            ptr += BE32(ptr) + 8;
        }
    }

    *output = kmalloc(*output_len);
    if (!*output) {
        return -ENOMEM;
    }

    uint8_t *outb = *output;
    while (inb < ((uint8_t *)input + input_len)) {
        size_t output_blk_sz = BE32(inb);
        inb += 4;

        if (output_blk_sz == 0) {
            /* End of blocks */
            break;
        }

        size_t input_blk_sz = BE32(inb);
        inb += 4;

        if (flags & (LZOP_FLAGS_UNCOMP_USE_ADLER32 | LZOP_FLAGS_UNCOMP_USE_CRC32)) {
            inb += 4;
        }

        if (output_blk_sz == input_blk_sz) {
            /* Uncompressed block */
            memcpy(outb, inb, input_blk_sz);
        } else {
            if (flags & (LZOP_FLAGS_COMP_USE_ADLER32 | LZOP_FLAGS_COMP_USE_CRC32)) {
                /* Compressed data checksum only included if data is actually compressed */
                inb += 4;
            }

            size_t output_sz = output_blk_sz;
            int ret = _lzo1x_decompress_block(inb, input_blk_sz, outb, &output_sz);
            if (ret) {
                kdebug(DEBUGSRC_MISC, ERR_WARN, "Block decompression failed\n");
                kfree(*output);
                *output = NULL;
                *output_len = 0;
                return ret;
            } else if (output_sz != output_blk_sz) {
                kdebug(DEBUGSRC_MISC, ERR_WARN, "Output block size does not match\n");
                ret = -EUNSPEC;
            }
        }

        inb += input_blk_sz;
        outb += output_blk_sz;
    }

    return 0;
}

/** Max value that when multiplied by 255 can fit within size_t */
#define MAX_255_COUNT       ((((size_t)~0) / 255) - 2)

#define INB_LEFT ((size_t)(in_end - inb))
#define OUTB_LEFT ((size_t)(out_end - outb))

#define CHECK_LB(LB) \
    if ((outb - (LB)) < (uint8_t *)output) { \
        kdebug(DEBUGSRC_MISC, ERR_WARN, "Lookbehind underrun: %d (%d)\n", ((uint8_t *)output - (outb - (LB))), (LB)); \
        return -EUNSPEC; \
    }

#define CHECK_OUT_LEFT(C) \
    if ((size_t)(out_end - outb) < (C)) { \
        kdebug(DEBUGSRC_MISC, ERR_WARN, "Output overrun\n"); \
        return -EUNSPEC; \
    }

#define CHECK_IN_LEFT(C) \
    if ((size_t)(in_end - inb) < (C)) { \
        kdebug(DEBUGSRC_MISC, ERR_WARN, "Input overrun\n"); \
        return -EUNSPEC; \
    }

#define SKIP_AND_COUNT_ZEROES() ({ \
    const uint8_t *pos = inb; \
                             \
    while (!*inb) {          \
        CHECK_IN_LEFT(1);    \
        inb++;               \
    }                        \
                             \
    size_t offset = inb - pos; \
    if (offset > MAX_255_COUNT) { \
        return -EUNSPEC;     \
    }                        \
                             \
    offset * 255;            \
})

/* For the time being, just use a dumb memcpy to make sure we don't run into
 * issues with overlapping look-behind copies. */
#define memcpy(D,S,L) { \
    for (size_t i = 0; i < L; i++) { \
        ((uint8_t *)(D))[i] = ((uint8_t *)(S))[i]; \
    } \
}

/**
 * @brief Decompress single LZO1X block
 *
 * @note This is not intended to be high performance, but rather more readable.
 * If decompression speed becomes a bottleneck in the future, it can be
 * addressed at that point.
 *
 * @param input Pointer to beginning of block
 * @param input_len Size of compressed data block
 * @param output Pointer at which to store output
 * @param output_len Pointer to size of output block, and where actual output
 * size is stored.
 *
 * @return 0 on success, negative error code on failure
 */
static int _lzo1x_decompress_block(const void *input, size_t input_len, void *output, size_t *output_len) {
    if (input_len < 3) {
        return -EUNSPEC;
    }

    const uint8_t *inb = input;
    const uint8_t *const in_end = &inb[input_len];

    uint8_t *outb = output;
    uint8_t *const out_end = &outb[*output_len];

    size_t lookbehind;

    size_t temp;
    size_t next;
    size_t state = 0;

    if (*inb == 17) {
        /* Linux extension for zero run-length-encoding unsupported */
        return -EUNSPEC;
    }

    if (*inb > 17) {
        temp = *inb++ - 17;
        if (temp < 4) {
            next = temp;
            goto skip_lookbehind;
        } else {
            goto copy_verbatim;
        }
    }

    while (1) {
        temp = *inb++;

        if (temp < 16) {
            if (state == 0) {
                if (temp == 0) {
                    temp = SKIP_AND_COUNT_ZEROES() + 15 + *inb++;
                }
                temp += 3;

copy_verbatim:
                CHECK_IN_LEFT(temp + 3);
                CHECK_OUT_LEFT(temp);

                memcpy(outb, inb, temp);
                inb  += temp;
                outb += temp;

                state = 4;
                continue;
            } else if (state == 4) {
                /* Insert 3 bytes from lookbehind */
                next = temp & 3;
                lookbehind = 1 + 0x0800 + (temp >> 2) + (*inb++ << 2);
                temp = 3;
            } else {
                /* Insert 2 bytes from lookbehind */
                next = temp & 3;
                lookbehind = 1 + (temp >> 2) + (*inb++ << 2);
                temp = 2;
            }
        } else if (temp < 32) {
            /* NOTE: Linux has an alternate variant here where it can encode runs
             * of zeroes efficiently, not implementing this for the time being, */

            lookbehind = (temp & 0x08) << 11;
            temp = (temp & 0x07) + 2;
            if (temp == 2) {
                temp += SKIP_AND_COUNT_ZEROES() + 7 + *inb++;
            }

            CHECK_IN_LEFT(2);
            next = LE16(inb);
            inb += 2;

            lookbehind += next >> 2;
            next &= 0x03;
            if (lookbehind == 0) {
                /* Done */
                break;
            }
            lookbehind += 0x4000;
        } else if (temp < 64) {
            temp = (temp & 0x1f) + 2;
            if (temp == 2) {
                temp += SKIP_AND_COUNT_ZEROES() + 31 + *inb++;
            }
            CHECK_IN_LEFT(2);
            next = LE16(inb);
            inb += 2;

            lookbehind = 1 + (next >> 2);
            next &= 3;
        } else {
            next = temp & 3;
            lookbehind = 1 + ((temp >> 2) & 7) + (*inb++ << 3);
            temp = (temp >> 5) + 1;
        }

        CHECK_LB(lookbehind);
        CHECK_OUT_LEFT(temp);

        memcpy(outb, outb - lookbehind, temp);
        outb += temp;

skip_lookbehind:
        CHECK_IN_LEFT(next + 3);
        CHECK_OUT_LEFT(next);

        memcpy(outb, inb, next);
        inb += next;
        outb += next;

        state = next;
    }

    if ((temp != 3) || (inb < in_end)) {
        return -EUNSPEC;
    }

    *output_len = (void *)outb - output;

    return 0;
}

