/** \file gtimer.h
 *  \brief Contains support for the ARMv7 (optional) generic timer.
 *
 */
#ifndef ARCH_ARM32_INTR_GTIMER_H
#define ARCH_ARM32_INTR_GTIMER_H

#include <hal/timer/timer.h>

#include <arch/registers.h>

void armv7_gtimer_create_timerdev(hal_timer_dev_t *dev);

int armv7_gtimer_init(uint32_t freq);

/** Clock frequency, in Hz */
#define __READ_CNTFRQ(VAR)  __mrc(VAR, p15, 0, c14, c0, 0)
#define __WRITE_CNTFRQ(VAR) __mcr(VAR, p15, 0, c14, c0, 0)

#define __READ_CNTHCTL(VAR)  __mrc(VAR, p15, 4, c14, c1, 0)
#define __WRITE_CNTHCTL(VAR) __mcr(VAR, p15, 4, c14, c1, 0)

#define __READ_CNTHP_CTL(VAR)  __mrc(VAR, p15, 4, c14, c2, 1)
#define __WRITE_CNTHP_CTL(VAR) __mcr(VAR, p15, 4, c14, c2, 1)

#define __READ_CNTHP_CVAL(VARL, VARH)  __mrrc(VARL, VARH, p15, 6, c14)
#define __WRITE_CNTHP_CVAL(VARL, VARH) __mcrr(VARL, VARH, p15, 6, c14)

#define __READ_CNTHP_TVAL(VAR)  __mrc(VAR, p15, 4, c14, c2, 0)
#define __WRITE_CNTHP_TVAL(VAR) __mcr(VAR, p15, 4, c14, c2, 0)

#define __READ_CNTKCTL(VAR)  __mrc(VAR, p15, 0, c14, c1, 0)
#define __WRITE_CNTKCTL(VAR) __mcr(VAR, p15, 0, c14, c1, 0)

#define __READ_CNTPCTL(VAR)  __mrc(VAR, p15, 0, c14, c2, 1)
#define __WRITE_CNTPCTL(VAR) __mcr(VAR, p15, 0, c14, c2, 1)

#define __READ_CNTP_CVAL(VARL, VARH)  __mrrc(VARL, VARH, p15, 2, c14)
#define __WRITE_CNTP_CVAL(VARL, VARH) __mcrr(VARL, VARH, p15, 2, c14)

#define __READ_CNTP_TVAL(VAR)  __mrc(VAR, p15, 0, c14, c2, 0)
#define __WRITE_CNTP_TVAL(VAR) __mcr(VAR, p15, 0, c14, c2, 0)

#define __READ_CNTPCT(VARL, VARH)  __mrrc(VARL, VARH, p15, 0, c14)

#define __READ_CNTV_CTL(VAR)  __mrc(VAR, p15, 0, c14, c3, 1)
#define __WRITE_CNTV_CTL(VAR) __mcr(VAR, p15, 0, c14, c3, 1)

#define __READ_CNTV_CVAL(VARL, VARH)  __mrrc(VARL, VARH, p15, 3, c14)
#define __WRITE_CNTV_CVAL(VARL, VARH) __mcrr(VARL, VARH, p15, 3, c14)

#define __READ_CNTV_TVAL(VAR)  __mrc(VAR, p15, 0, c14, c3, 0)
#define __WRITE_CNTV_TVAL(VAR) __mcr(VAR, p15, 0, c14, c3, 0)

#define __READ_CNTVCT(VARL, VARH)  __mrrc(VARL, VARH, p15, 1, c14)

#define __READ_CNTVOFF(VARL, VARH)  __mrrc(VARL, VARH, p15, 4, c14)
#define __WRITE_CNTVOFF(VARL, VARH) __mcrr(VARL, VARH, p15, 4, c14)

#endif