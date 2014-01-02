#ifndef IPC_H
#define IPC_H

#include <types.h>

int send_message(int dest, void *msg, int size);

int recv_message(void *msg, int size);


#endif