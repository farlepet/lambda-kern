#include <intr/intr.h>
#include <time/time.h>

#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>

extern uint32_t __int_table[];

static uint32_t *vec_table = (uint32_t *)&__int_table;

static armv7_gic_handle_t *__gic = NULL;

void intr_set_handler(interrupt_idx_e idx, void *ptr) {
    if (idx >= 8) {
        return;
    }

    vec_table[idx] = 0xEA000000 | (((uint32_t)ptr - (4 * (idx + 2))) >> 2);
}

void intr_attach_gic(armv7_gic_handle_t *hand) {
    __gic = hand;
}

extern void (*gtimer_callback)(void);

__hot
__attribute__((interrupt ("IRQ")))
static void intr_irq_handler(void) {
    __INTR_BEGIN;

    if(__gic) {
        armv7_gic_irqhandle(__gic);
    }

    uint32_t tmp;
    __READ_CNTV_CTL(tmp);
    if (tmp & (1UL << 2)) {
        /* Timer interrupt has fired. */
        /* Flip transition direction to free interrupt? Needs testing. */
        /*__READ_CNTKCTL(tmp);
        tmp ^= (1UL << 3);
        __WRITE_CNTKCTL(tmp);*/

        /* Clear CompareValue register */
        tmp = 0;
        __WRITE_CNTV_CVAL(tmp, tmp);

        kerneltime++;

        if(gtimer_callback) {
            gtimer_callback();
        }
    }

    __INTR_END;
}

__hot
__attribute__((interrupt ("FIQ")))
static void intr_fiq_handler(void) {
    __INTR_BEGIN;
    /* TODO? */
    __INTR_END;
}

__hot
__attribute__((interrupt))
static void intr_stub_handler(void) {
    __INTR_BEGIN;
    /* TODO? */
    __INTR_END;
}

void intr_init(void) {
    __WRITE_VBAR(vec_table);

    intr_set_handler(INT_RESET,         intr_stub_handler);
    intr_set_handler(INT_UNDEFINED,     intr_stub_handler);
    intr_set_handler(INT_SYSCALL,       intr_stub_handler);
    intr_set_handler(INT_PREFETCHABORT, intr_stub_handler);
    intr_set_handler(INT_DATAABORT,     intr_stub_handler);
    intr_set_handler(INT_HYPTRAP,       intr_stub_handler);
    intr_set_handler(INT_IRQ,           intr_irq_handler);
    intr_set_handler(INT_FIQ,           intr_fiq_handler);

    uint32_t temp;
    __READ_SCTLR(temp);
    temp &= ~(1UL << 24); /* VE */
    __WRITE_SCTLR(temp);
}