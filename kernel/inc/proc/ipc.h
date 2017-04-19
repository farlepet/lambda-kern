#ifndef IPC_H
#define IPC_H

#include <types.h>

/**
 * Defines an internal representation of a message between processes
 */
struct ipc_message {
    uint32_t message_id; ///< Message ID
    int      src_pid;    ///< PID of sending process
    int      dest_pid;   ///< PID of receiving process
    uint32_t length;     ///< Length of message, in bytes
    void    *message;    ///< Pointer to message, in kernel space
};

/**
 * Defines a representation of a message that processes themselves may see.
 */
struct ipc_message_user {
    uint32_t message_id; ///< Message ID
    int      src_pid;    ///< PID of sending process
    uint32_t length;     ///< Length of message
}

int send_message(int dest, void *msg, int size);

int recv_message(void *msg, int size);

#endif