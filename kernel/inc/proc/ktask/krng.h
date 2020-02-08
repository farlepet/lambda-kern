#ifndef KTASK_KRNG_H
#define KTASK_KRNG_H

#include <types.h>

__noreturn void krng_task(void);

/**
 * Types of KRNG messages
 */
enum krng_types
{
	KRNG_REQUEST     = 0, //!< Random number(s) request
	KRNG_ADD_ENTROPY = 1  //!< Add entropy to system
};


/**
 * Message that contains KRNG message type
 */
struct rng_type_msg
{
	uint8_t type; //!< Type of message
};

/**
 * Message requesting a number of random bytes.
 */
struct rng_request_msg
{
	uint32_t n_bytes; //!< Number of bytes requested
};

/**
 * Message adding entropy to RNG
 */
struct rng_add_entropy_msg
{
	uint32_t n_bytes; //!< Number of bytes in message
	uint8_t  bytes[]; //!< Bytes
};

/**
 * Combines type and request messages
 */
struct rng_type_request_msg
{
	struct rng_type_msg    rtm; //!< Type message
	struct rng_request_msg rrm; //!< Request message
};

/**
 * Combines type and entropy addition messages
 */
struct rng_type_add_entropy_msg
{
	struct rng_type_msg        rtm; //!< Type message
	struct rng_add_entropy_msg raem; //!< Entropy data message
};

/**
 * Add entropy to RNG
 *
 * @param ent Byte to add
 */
void krng_add_entropy(uint8_t ent);

#endif // KTASK_KRNG_H