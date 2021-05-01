#include <lambda/export.h>
#include <data/cbuff.h>

int cbuff_put(uint8_t data, struct cbuff *buff) {
	if(!buff || !buff->buff) return CBUFF_INVAL; // Invalid buffer
	if(buff->count >= buff->size) return CBUFF_FULL; // Not enough room in cbuff

	buff->buff[buff->head] = data;

	buff->head++;
	if(buff->head == buff->size) buff->head = 0;

	buff->count++;

	return 0;
}
EXPORT_FUNC(cbuff_put);

int cbuff_get(struct cbuff *buff) {
	if(!buff || !buff->buff) return CBUFF_INVAL; // Invalid buffer
	if(buff->count == 0) return CBUFF_EMPTY; // No data to be read

	uint8_t d = buff->buff[buff->tail];

	buff->tail++;
	if(buff->tail == buff->size) buff->tail = 0;

	buff->count--;

	return d;
}
EXPORT_FUNC(cbuff_get);


int cbuff_write(uint8_t *data, int size, struct cbuff *buff) {
	//kerror(ERR_INFO, "write_cbuff: count = %d size = %d", buff->count, size);
	if(!data) return CBUFF_INVLD; // Invalid data
	if(!buff || !buff->buff) return CBUFF_INVAL; // Invalid buffer
	if(size > buff->size) return CBUFF_FULL; // Not enough room in buffer
	if(size > (buff->size - buff->count)) return CBUFF_FULL; // Also not enough room in the buffer

	int i = 0;
	while(size--) {
		int err = cbuff_put(data[i++], buff);
		if(err) return err;
	}

	return 0;
}
EXPORT_FUNC(cbuff_write);

int cbuff_read(uint8_t *data, int size, struct cbuff *buff) {
	//kerror(ERR_INFO, "read_cbuff: count = %d size = %d", buff->count, size);
	if(!data) return CBUFF_INVLD; // Invalid data
	if(!buff || !buff->buff) return CBUFF_INVAL; // Invalid buffer
	if(size > buff->size) return CBUFF_NENOD; // Not enough readable data
	if(size > buff->count) return CBUFF_NENOD; // Not enough readable data

	int i = 0;
	while(size--) {
		int err = cbuff_get(buff);
		if((uint32_t)err & 0xFFFFFF00) return err;
		data[i++] = (uint8_t)err;
	}

	return 0;
}
EXPORT_FUNC(cbuff_read);
