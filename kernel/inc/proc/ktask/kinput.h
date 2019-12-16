#ifndef KTASK_KINPUT_H
#define KTASK_KINPUT_H

#include <types.h>

#define KINPUT_MAX_SUBS 8 ///< Maximum number of processes subscribed to kinput


__noreturn void kinput_task(void);

#endif // KTASK_KINPUT_H