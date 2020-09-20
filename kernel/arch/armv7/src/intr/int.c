#include <intr/intr.h>

#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>

static uint32_t *vec_table = (uint32_t *)0x00000000;

void intr_set_handler(interrupt_idx_e idx, void *ptr) {
    if (idx >= 8) {
        return;
    }

    vec_table[idx] = 0xEA000000 | (((uint32_t)ptr - (4 * (idx + 2))) >> 2);
}

__unused
__attribute__((interrupt ("IRQ")))
static void intr_irq_handler() {
    uint32_t tmp;
    __READ_CNTV_CTL(tmp);
    if (tmp & (1UL << 2)) {
        /* Timer interrupt has fired. */
        /* Flip transition direction to free interrupt? Needs testing. */
        __READ_CNTKCTL(tmp);
        tmp ^= (1UL << 3);
        __WRITE_CNTKCTL(tmp);

        /* TODO: Task switch */
    }
}