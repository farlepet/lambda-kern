#ifndef KTASK_KINPUT_H
#define KTASK_KINPUT_H

#include <types.h>

extern struct kfile *kinput_dest;

__noreturn void kinput_task(void);

#endif // KTASK_KINPUT_H