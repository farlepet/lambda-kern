#ifndef CBUFF_H
#define CBUFF_H

#include <types.h>

#define CBUFF_SIZE 512

typedef struct cbuff //!< A FIFO circular buffer
{ // TODO: Remove either tail or count, they are redundant
	int head;  //!< Index of last written byte
	int tail;  //!< Index of first written byte
	int count; //!< Number of readable bytes in the buffer
	int size;  //!< Size of the buffer
	uint8_t *buff;  //!< The buffer
} __align(8) cbuff_t; // For faster access times

// Errors returned by cbuff functions
#define CBUFF_FULL   0x70000000 // Buffer is full
#define CBUFF_INVAL  0x60000000 // Buffer is invalid
#define CBUFF_INVLD  0x50000000 // Invalid data
#define CBUFF_EMPTY  0x40000000 // Buffer is empty
#define CBUFF_NENOD  0x30000000 // Not enough data in buffer to satisfy the request
#define CBUFF_ERRMSK 0xF0000000 // Where error bits are located

#define STATIC_CBUFF(SZ) { 0, 0, 0, (SZ), (uint8_t[(SZ)]){ 0, }}


int put_cbuff(uint8_t data, struct cbuff *buff);

int get_cbuff(struct cbuff *buff);


int write_cbuff(uint8_t *data, int size, struct cbuff *buff);

int read_cbuff(uint8_t *data, int size, struct cbuff *buff);


#endif // CBUFF_H