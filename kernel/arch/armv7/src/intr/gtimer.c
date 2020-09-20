#include <arch/intr/gtimer.h>

int armv7_gtimer_init(uint32_t freq) {
    uint32_t tmp;
    
    __WRITE_CNTFRQ(freq);

    __READ_CNTKCTL(tmp);
    tmp |= (1UL << 1) | /* Enable PL0 access to Vitual timer */
           (1UL << 2) | /* Enable event */
           (8UL << 4) | /* Trigger event on 8th bit of CNTVCT */
           (1UL << 8);  /* Enable PL0 access to Vitual timer */
    __WRITE_CNTKCTL(tmp);

    __READ_CNTV_CTL(tmp);
    tmp |= (1UL << 0); /* Enable virtual timer */
    __WRITE_CNTV_CTL(tmp);

    return 0;
}