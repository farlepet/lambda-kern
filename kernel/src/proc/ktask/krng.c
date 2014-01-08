#include <proc/ktasks.h>
#include <proc/ipc.h>
#include <mm/cbuff.h>

/*
 * This is an extremely horrible random number generator implementation.
 * While it DOES use some input from HID's, it doesn't do a whole lot to
 * modify that data.
 */

static u8 buff[0x1000];
static struct cbuff e_pool = { 0, 0, 0, 0x1000, buff };


static int seed[4] = { 213424, 464854864, 86445648, 21681121 }; //!< Some arbitrary numbers...

u8 generate_rand_byte(void);

/**
 * Kernel (Pseudo-)Random Number Generator (RNG)
 */
__noreturn void krng_task()
{
	ktask_pids[KRNG_TASK_SLOT] = current_pid;

	for(;;)
	{
		struct rng_request rngr;
		recv_message(&rngr, sizeof(struct rng_request));

		while(rngr.n_bytes)
		{
			u8 byte = generate_rand_byte();
			send_message(rngr.pid, &byte, 1);
		}
	}
}

void rot_seed(int a, int b, int c, int d)
{
	int _a = seed[0];
	int _b = seed[1];
	int _c = seed[2];
	int _d = seed[3];
	seed[a] = _a;
	seed[b] = _b;
	seed[c] = _c;
	seed[d] = _d;
}

void krng_add_entropy(u8 ent)
{
	u32 entropy = (u32)(ent | ((ent << 8) ^ seed[3]) | ((ent << 16) ^ seed[0]) | ((ent << 24) ^ seed[2]));
	entropy *= (u32)seed[2];
	rot_seed(0, 3, 2, 1);

#if __has_builtin(__builtin_readcyclecounter) // Add a bit extra
		seed[3] = (int)__builtin_readcyclecounter();
#endif

	ent = (entropy & 0x03) | (entropy >> 8 & 0x0C) | (entropy >> 16 & 0x30) | (entropy >> 24 & 0xC0); // Shuffle their values about
	put_cbuff(ent, &e_pool);
}

u8 generate_rand_byte()
{
	rot_seed(2, 1, 3, 0);
	u32 i = 0xFFFF;
	while(i & 0xFFFFFF00) i = (u32)get_cbuff(&e_pool);
	return (u8)i ^ (u8)seed[0] >> (u8)(i & 0x0F);
}
