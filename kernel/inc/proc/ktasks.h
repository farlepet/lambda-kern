#ifndef KTASKS_H
#define KTASKS_H

#include <proc/ktask/kinput.h>
#include <proc/ktask/kterm.h>
#include <proc/ktask/idle.h>
#include <proc/ktask/kbug.h>
#include <proc/mtask.h>
#include <types.h>

/*
 * NOTE: These must NEVER be renoved or modified/rordered as to maintain backwards compatibility
 */

#define IDLE_TASK_SLOT   0
#define KBUG_TASK_SLOT   1
#define KINPUT_TASK_SLOT 2
#define KTERM_TASK_SLOT  3

#define KTASK_SLOTS      4

extern int ktask_pids[KTASK_SLOTS];

void init_ktasks(void);


#endif
