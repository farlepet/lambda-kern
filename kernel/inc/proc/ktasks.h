#ifndef KTASKS_H
#define KTASKS_H

#include <proc/ktask/kinput.h>
#include <proc/ktask/idle.h>
#include <proc/ktask/kbug.h>
#include <proc/ktask/kvid.h>
#include <proc/ktask/krng.h>
#include <proc/mtask.h>
#include <types.h>

#define IDLE_TASK_SLOT   0
#define KVID_TASK_SLOT   1
#define KBUG_TASK_SLOT   2
#define KINPUT_TASK_SLOT 3
#define KRNG_TASK_SLOT   4

#define KTASK_SLOTS    5

int ktask_pids[KTASK_SLOTS];


void init_ktasks(void);


#endif