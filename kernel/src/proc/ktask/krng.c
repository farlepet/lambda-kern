#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <mm/cbuff.h>
#include <mm/alloc.h>

/*
 * This is an extremely horrible random number generator implementation.
 * While it DOES use some input from HID's, it doesn't do a whole lot to
 * modify that data.
 */

static uint8_t buff[0x1000];
static struct cbuff e_pool = { 0, 0, 0, 0x1000, buff };


static int seed[4] = { 213424, 464854864, 86445648, 21681121 }; //!< Some arbitrary numbers...

uint8_t generate_rand_byte(void);

/**
 * Kernel (Pseudo-)Random Number Generator (RNG)
 */
__noreturn void krng_task()
{
	ktask_pids[KRNG_TASK_SLOT] = current_pid;

	for(;;)
	{
		int ret;
		struct ipc_message_user umsg;
		while((ret = ipc_user_recv_message_blocking(&umsg)) < 0)
		{
			kerror(ERR_MEDERR, "KRNG: IPC error: %d", ret);
		}
		
		void *data = kmalloc(umsg.length);

		ipc_user_copy_message(umsg.message_id, data);

		switch(((struct rng_type_msg *)data)->type)
		{
			case KRNG_REQUEST:
			{
				struct rng_type_request_msg *m = (struct rng_type_request_msg *)data;
				uint8_t *bytes = kmalloc(m->rrm.n_bytes);

				for(uint32_t i = 0; i < m->rrm.n_bytes; i++)
				{
					bytes[i] = generate_rand_byte();
				}

				ipc_user_create_and_send_message(umsg.src_pid, bytes, m->rrm.n_bytes);

				kfree(bytes);
			} break;

			case KRNG_ADD_ENTROPY:
			{
				struct rng_type_add_entropy_msg *m = (struct rng_type_add_entropy_msg *)data;

				for(uint32_t i = 0; i < m->raem.n_bytes; i++)
				{
					krng_add_entropy(m->raem.bytes[i]);
				}
			} break;
		}

		kfree(data);
	}
}

static void rot_seed(int a, int b, int c, int d)
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

/**
 * A Galois LFSR
 *
 * @param n input number
 * @returns output number
 */
static uint32_t glfsr(uint32_t n)
{
	return (n >> 1) ^ (-(n & 1) & 0xD0000001);
}

void krng_add_entropy(uint8_t ent)
{
	uint32_t entropy = (uint32_t)(ent | ((ent << 8) ^ seed[3]) | ((ent << 16) ^ seed[0]) | ((ent << 24) ^ seed[2]));
	entropy *= (uint32_t)seed[2];
	rot_seed(0, 3, 2, 1);

#if __has_builtin(__builtin_readcyclecounter) // Add a bit extra
		seed[3] = (int)__builtin_readcyclecounter();
#endif

	seed[0] = (int)glfsr((uint32_t)seed[0]);
	uint32_t t = (uint32_t)seed[2];
	seed[2] = (int)glfsr((uint32_t)seed[1]);
	seed[1] = (int)glfsr(t);

	ent = (entropy & 0x03) | (entropy >> 8 & 0x0C) | (entropy >> 16 & 0x30) | (entropy >> 24 & 0xC0); // Shuffle their values about
	put_cbuff(ent, &e_pool);
}

uint8_t generate_rand_byte()
{
	rot_seed(2, 1, 3, 0);
	uint32_t i = 0xFFFF;
	while(i & 0xFFFFFF00) i = (uint32_t)get_cbuff(&e_pool);
	return (uint8_t)i ^ (uint8_t)seed[0] >> (uint8_t)(i & 0x0F);
}
