#ifndef DRV_DRIVER_H
#define DRV_DRIVER_H

#include <lambda/drv/driver.h>
#include <fs/kfile.h>

int driver_read(struct kfile *file, lambda_drv_head_t **head, uintptr_t *base);

#endif