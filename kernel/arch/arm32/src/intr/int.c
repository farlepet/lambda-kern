#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <err/panic.h>

#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>

extern uint32_t __int_table[];

void intr_handler(uint32_t, uintptr_t);

static struct {
    void (*intr_handlers[INTR_MAX])(uint8_t, uintptr_t);

    void (*irqhandler)(void *);
    void *irqdata;

    uint32_t *vbar;
} _intr_state = {
    .vbar = (uint32_t *)&__int_table
};

void intr_set_handler(interrupt_idx_e idx, void (*handler)(uint8_t, uintptr_t)) {
    if (idx >= INTR_MAX) {
        return;
    }

    _intr_state.intr_handlers[idx] = handler;
}

void intr_attach_irqhandler(void (*handler)(void *), void *data) {
    _intr_state.irqhandler = handler;
    _intr_state.irqdata    = data;
}

__hot
static void intr_irq_handler(uint8_t __unused intn, uintptr_t __unused lr) {
    if(_intr_state.irqhandler) {
        _intr_state.irqhandler(_intr_state.irqdata);
    }
}

__hot
static void intr_fiq_handler(uint8_t __unused intn, uintptr_t __unused lr) {
    /* TODO? */
}

__hot
__noreturn
static void intr_stub_handler(uint8_t intn, uintptr_t lr) {
    static const char *int_name[INTR_MAX] = {
        [INTR_RESET]         = "INT_RESET",
        [INTR_UNDEFINED]     = "INT_UNDEFINED",
        [INTR_SYSCALL]       = "INT_SYSCALL",
        [INTR_PREFETCHABORT] = "INT_PREFETCHABORT",
        [INTR_DATAABORT]     = "INT_DATAABORT",
        [INTR_HYPTRAP]       = "INT_HYPTRAP",
        [INTR_IRQ]           = "INT_IRQ",
        [INTR_FIQ]           = "INT_FIQ"
    };

    if(intn < INTR_MAX) {
        kpanic("intr_stub_handler(%s, %08X)\n", int_name[intn], lr);
    } else {
        kpanic("intr_stub_handler(%d, %08X)\n", intn, lr);
    }
}

__hot
void intr_handler(uint32_t intn, uintptr_t lr) {
    if(intn >= INT_MAX) {
        kpanic("intr_handler(): intn out of range: %u", intn);
    }

    if(_intr_state.intr_handlers[intn]) {
        _intr_state.intr_handlers[intn]((uint8_t)intn, lr);
    }
}

void intr_init(void) {
    __WRITE_VBAR(_intr_state.vbar);

    intr_set_handler(INTR_RESET,         intr_stub_handler);
    intr_set_handler(INTR_UNDEFINED,     intr_stub_handler);
    intr_set_handler(INTR_SYSCALL,       intr_stub_handler);
    intr_set_handler(INTR_PREFETCHABORT, intr_stub_handler);
    intr_set_handler(INTR_DATAABORT,     intr_stub_handler);
    intr_set_handler(INTR_HYPTRAP,       intr_stub_handler);
    intr_set_handler(INTR_IRQ,           intr_irq_handler);
    intr_set_handler(INTR_FIQ,           intr_fiq_handler);

    uint32_t temp;
    __READ_SCTLR(temp);
    temp &= ~(1UL << 24); /* VE */
    __WRITE_SCTLR(temp);
}