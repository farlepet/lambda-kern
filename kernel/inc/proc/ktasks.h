#ifndef KTASKS_H
#define KTASKS_H

#include <proc/ktask/kinput.h>
#include <proc/ktask/kterm.h>
#include <proc/ktask/kbug.h>
#include <proc/mtask.h>
#include <types.h>

#define KINPUT_TASK_SLOT 0
#define KTERM_TASK_SLOT  1
#define KBUG_TASK_SLOT   2

#define KTASK_SLOTS      3

extern int ktask_pids[KTASK_SLOTS];

void init_ktasks(void);


#endif
