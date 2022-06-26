#include <lambda/export.h>
#include <data/cbuff.h>

#define WRAP(N, M) (((N) >= (M)) ? ((N) - (M)) : (N))

int cbuff_put(uint8_t data, cbuff_t *buff) {
    if(!buff || !buff->buff) return CBUFF_ERR_INVAL; // Invalid buffer
    if(buff->count >= buff->size) return CBUFF_ERR_FULL; // Not enough room in cbuff

    buff->buff[WRAP(buff->begin + buff->count, buff->size)] = data;

    buff->count++;

    return 0;
}
EXPORT_FUNC(cbuff_put);

int cbuff_get(cbuff_t *buff) {
    if(!buff || !buff->buff) return CBUFF_ERR_INVAL; // Invalid buffer
    if(buff->count == 0) return CBUFF_ERR_EMPTY; // No data to be read

    uint8_t d = buff->buff[buff->begin];

    buff->begin = WRAP(buff->begin + 1, buff->size);
    buff->count--;

    return d;
}
EXPORT_FUNC(cbuff_get);


int cbuff_write(const uint8_t *data, size_t size, cbuff_t *buff) {
    if(!data) return CBUFF_ERR_INVLD; // Invalid data
    if(!buff || !buff->buff) return CBUFF_ERR_INVAL; // Invalid buffer
    if(size > (buff->size - buff->count)) return CBUFF_ERR_FULL; // Also not enough room in the buffer

    int i = 0;
    while(size--) {
        int err = cbuff_put(data[i++], buff);
        if(err) return err;
    }

    return 0;
}
EXPORT_FUNC(cbuff_write);

int cbuff_read(uint8_t *data, size_t size, cbuff_t *buff) {
    if(!data) return CBUFF_ERR_INVLD; // Invalid data
    if(!buff || !buff->buff) return CBUFF_ERR_INVAL; // Invalid buffer
    if(size > buff->count) return CBUFF_ERR_NENOD; // Not enough readable data

    int i = 0;
    while(size--) {
        int err = cbuff_get(buff);
        if((uint32_t)err & 0xFFFFFF00) return err;
        data[i++] = (uint8_t)err;
    }

    return 0;
}
EXPORT_FUNC(cbuff_read);
