#ifndef IPC_H
#define IPC_H

#include <types.h>

int send_message(int dest, u8 *msg, int size);

int recv_message(u8 *msg, int size);


#endif