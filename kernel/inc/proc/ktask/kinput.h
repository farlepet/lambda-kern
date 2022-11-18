#ifndef KTASK_KINPUT_H
#define KTASK_KINPUT_H

#include <fs/kfile.h>

extern kfile_hand_t *kinput_dest;

__noreturn void kinput_task(void);

#endif // KTASK_KINPUT_H
