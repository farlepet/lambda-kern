#ifndef IPC_H
#define IPC_H

#include <types.h>

#define IPC_MAX_MESSAGES 512

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
};


extern struct ipc_message *ipc_messages[IPC_MAX_MESSAGES];


int send_message(int dest, void *msg, int size);

int recv_message(void *msg, int size);



/**
 * Creates a user message from an internal IPC message
 *
 * @param msg Pointer to internal message structure containing data
 * @param umsg Pointer to user message structure to fill
 * @return < 0 on error
 */
int ipc_create_user_message(struct ipc_message *msg, struct ipc_message_user *umsg);

/**
 * Creates a new message with the supplied parameters
 *
 * Allocates kernel memory of given length and copies the message contents into it.
 *
 * @param msg Pointer to internal message structure to fill
 * @param src PID of source process
 * @param dest PID of destination process
 * @param message Message contents
 * @param length Length of message, in bytes
 * @return < 0 on error
 */
int ipc_create_message(struct ipc_message *msg, int src, int dest, void *message, uint32_t length);

/**
 * Deletes the messsage with the given ID
 *
 * Deallocates memory used for message data.
 *
 * @param msg Message to delete
 * @return < 0 on error
 */
int ipc_delete_message(struct ipc_message *msg);

/**
 * Copys data from message into destination
 *
 * @param msg Message to copy data from
 * @param dest Pointer to destination of data
 * @return < 0 on error
 */
int ipc_copy_message_data(struct ipc_message *msg, void *dest);

#endif