#include <err/error.h>
#include <mm/cbuff.h>





#include <video.h>

int put_cbuff(u8 data, struct cbuff *buff)
{
	if(!buff->buff | !buff) return CBUFF_INVAL; // Invalid buffer
	if(buff->count >= buff->size) return CBUFF_FULL; // Not enough room in cbuff

	buff->buff[buff->head] = data;

	if(buff->head == buff->size) buff->head = 0;
	else                         buff->head++;

	buff->count++;

	return 0;
}

int get_cbuff(struct cbuff *buff)
{
	if(!buff->buff || !buff) return CBUFF_INVAL; // Invalid buffer
	if(buff->count == 0) return CBUFF_EMPTY; // No data to be read

	u8 d = buff->buff[buff->tail];

	if(buff->tail == buff->size) buff->tail = 0;
	else                         buff->tail++;

	buff->count--;

	return d;
}


int write_cbuff(u8 *data, int size, struct cbuff *buff)
{
	if(!data) return CBUFF_INVLD; // Invalid data
	if(!buff->buff | !buff) return CBUFF_INVAL; // Invalid buffer
	if(size > buff->size) return CBUFF_FULL; // Not enough room in buffer
	if(size > (buff->size - buff->count)) return CBUFF_FULL; // Also not enough room in the buffer

	int i = 0;
	while(size--)
	{
		int err = put_cbuff(data[i++], buff);
		if(err) return err;
	}

	return 0;
}

int read_cbuff(u8 *data, int size, struct cbuff *buff)
{
	if(!data) return CBUFF_INVLD; // Invalid data
	if(!buff->buff | !buff) return CBUFF_INVAL; // Invalid buffer
	if(size > buff->size) return CBUFF_NENOD; // Not enough readable data
	if(size > buff->count) return CBUFF_NENOD; // Not enough readable data

	int i = 0;
	while(size--)
	{
		int err = get_cbuff(buff);
		if(err & 0xFFFFFF00) return err;
		data[i++] = (u8)err;
	}

	return 0;
}