#ifndef KTASK_KRNG_H
#define KTASK_KRNG_H

#include <types.h>

void krng_task(void);

enum krng_types
{
	KRNG_REQUEST     = 0,
	KRNG_ADD_ENTROPY = 1
};


struct rng_type_msg
{
	u8 type;
};

struct rng_request_msg
{
	u32 n_bytes;
};

struct rng_add_entropy_msg
{
	u32 n_bytes;
	u8  bytes[];
};


struct rng_type_request_msg
{
	struct rng_type_msg    rtm;
	struct rng_request_msg rrm;
};

struct rng_type_add_entropy_msg
{
	struct rng_type_msg        rtm;
	struct rng_add_entropy_msg raem;
};

void krng_add_entropy(u8 ent);

#endif // KTASK_KRNG_H