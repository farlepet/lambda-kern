#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <err/panic.h>

#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>

extern uint32_t __int_table[];
static uint32_t *vec_table = (uint32_t *)&__int_table;

static void (*__intr_handlers[INT_MAX])(uint8_t, uintptr_t);

static armv7_gic_handle_t *__gic = NULL;

void intr_set_handler(interrupt_idx_e idx, void (*handler)(uint8_t, uintptr_t)) {
    if (idx >= INT_MAX) {
        return;
    }

    __intr_handlers[idx] = handler;
}

void intr_attach_gic(armv7_gic_handle_t *hand) {
    __gic = hand;
}

extern void (*gtimer_callback)(void);

__hot
static void intr_irq_handler(uint8_t __unused intn, uintptr_t __unused lr) {
    if(__gic) {
        armv7_gic_irqhandle(__gic);
    }

#if 0
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
#endif
}

__hot
static void intr_fiq_handler(uint8_t __unused intn, uintptr_t __unused lr) {
    /* TODO? */
}

__hot
static void intr_stub_handler(uint8_t __unused intn, uintptr_t __unused lr) {
    /* TODO? */
}

__hot
void intr_handler(uint32_t intn, uintptr_t lr) {
    if(intn >= INT_MAX) {
        kpanic("intr_handler(): intn out of range: %u", intn);
    }

    if(__intr_handlers[intn]) {
        __intr_handlers[intn]((uint8_t)intn, lr);
    }
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