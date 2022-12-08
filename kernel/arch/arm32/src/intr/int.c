#include <stddef.h>

#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/mtask.h>

#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>

extern uint32_t __int_table[];

void intr_handler(uint32_t, arm32_intr_frame_t *);

static struct {
    intr_handler_hand_t *hdlrs[INTR_MAX];

    void (*irqhandler)(void *);
    void *irqdata;

    uint32_t *vbar;
} _intr_state = {
    .vbar = (uint32_t *)&__int_table
};

void intr_set_handler(interrupt_idx_e idx, intr_handler_hand_t *hdlr) {
    if (idx >= INTR_MAX) {
        return;
    }

    _intr_state.hdlrs[idx] = hdlr;
}

void intr_attach_irqhandler(void (*handler)(void *), void *data) {
    _intr_state.irqhandler = handler;
    _intr_state.irqdata    = data;
}

__hot
static void _intr_irq_handler(intr_handler_hand_t *hdlr __unused) {
    if(_intr_state.irqhandler) {
        _intr_state.irqhandler(_intr_state.irqdata);
    }
}

__hot
static void _intr_fiq_handler(intr_handler_hand_t *hdlr __unused) {
    /* TODO? */
}

__hot
__noreturn
static void _intr_stub_handler(intr_handler_hand_t *hdlr) {
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

    unsigned intn = hdlr->arch.int_n;

    if(intn < INTR_MAX) {
        kpanic("intr_stub_handler(%s, %08X)\n", int_name[intn], hdlr->arch.frame->lr);
    } else {
        kpanic("intr_stub_handler(%d, %08X)\n", intn, hdlr->arch.frame->lr);
    }
}

__hot
void intr_handler(uint32_t intn, arm32_intr_frame_t *frame) {
    if(intn >= INTR_MAX) {
        kpanic("intr_handler(): intn out of range: %u", intn);
    }

    kthread_t *thread = mtask_get_curr_thread();
    if(thread) {
        thread->arch.int_frame = frame;
    }

    if(_intr_state.hdlrs[intn] &&
       _intr_state.hdlrs[intn]->callback) {
        _intr_state.hdlrs[intn]->arch.int_n = intn;
        _intr_state.hdlrs[intn]->arch.frame = frame;
        _intr_state.hdlrs[intn]->callback(_intr_state.hdlrs[intn]);
    }
}

static intr_handler_hand_t _stub_int_hdlr = {
    .callback = _intr_stub_handler,
    .data     = NULL
};
static intr_handler_hand_t _irq_int_hdlr = {
    .callback = _intr_irq_handler,
    .data     = NULL
};
static intr_handler_hand_t _fiq_int_hdlr = {
    .callback = _intr_fiq_handler,
    .data     = NULL
};

void intr_init(void) {
    __WRITE_VBAR(_intr_state.vbar);

    intr_set_handler(INTR_RESET,         &_stub_int_hdlr);
    intr_set_handler(INTR_UNDEFINED,     &_stub_int_hdlr);
    intr_set_handler(INTR_SYSCALL,       &_stub_int_hdlr);
    intr_set_handler(INTR_PREFETCHABORT, &_stub_int_hdlr);
    intr_set_handler(INTR_DATAABORT,     &_stub_int_hdlr);
    intr_set_handler(INTR_HYPTRAP,       &_stub_int_hdlr);
    intr_set_handler(INTR_IRQ,           &_irq_int_hdlr);
    intr_set_handler(INTR_FIQ,           &_fiq_int_hdlr);

    uint32_t temp;
    __READ_SCTLR(temp);
    temp &= ~(1UL << 24); /* VE */
    __WRITE_SCTLR(temp);
}

