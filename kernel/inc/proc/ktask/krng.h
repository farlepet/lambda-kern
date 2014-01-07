#ifndef KTASK_KRNG_H
#define KTASK_KRNG_H

#include <types.h>

void krng_task(void);

struct rng_request
{
	int pid;     //!< PID requesting the random numbers
	u32 n_bytes; //!< Number of bytes requested
};

void krng_add_entropy(u8 ent);

#endif // KTASK_KRNG_H