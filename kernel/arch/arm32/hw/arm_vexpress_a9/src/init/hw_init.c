#include <arch/init/hw_init.h>
#include <arch/io/uart/uart_pl011.h>
#include <arch/plat/platform.h>
#include <arch/intr/gtimer.h>
#include <arch/intr/timer/timer_sp804.h>
#include <arch/intr/int.h>
#include <arch/proc/tasking.h>

#include <err/error.h>
#include <mm/alloc.h>
#include <io/output.h>

hal_intctlr_dev_t          intctlr;

static uart_pl011_handle_t _pl011;
static hal_io_char_dev_t   _uart;
static hal_clock_dev_t     _uartclock;

int hw_init_console(void) {
    _uartclock.freq = 48000000; /* TODO */
    uart_pl011_init(&_pl011,
                    VEXPRESS_A9_PERIPH_UART0_BASE,
                    &_uartclock,
                    115200);
    uart_pl011_create_chardev(&_pl011, &_uart);
    output_set_dev(&_uart);

    return 0;
}

static armv7_gic_handle_t _gic;
int hw_init_interrupts(void) {
    uintptr_t mpcore;
    __READ_PERIPHBASE(mpcore);
    kerror(ERR_INFO, "PERIPHBASE: %08X", mpcore);

    armv7_gic_init(&_gic,
                  (void *)(mpcore + MPCORE_PERIPHBASE_OFF_ICC),
                  (void *)(mpcore + MPCORE_PERIPHBASE_OFF_DCU));
    armv7_gic_create_intctlrdev(&_gic, &intctlr);
    kerror(ERR_INFO, "GIC Initialized");

    uart_pl011_int_attach(&_pl011, &intctlr, VEXPRESSA9_INT_UART0);

    return 0;
}

int hw_init_mm(void) {
    /* TODO: Abstract this. Make memory map partially user-configurable. */
    init_alloc(VEXPRESSA9_ALLOC_BASE, VEXPRESSA9_ALLOC_SIZE);

    return 0;
}

__hot
static void _clk_count() {
    /* TODO: Determine from timer struct */
    time_update(10000000);
}

__hot
static void _task_switch_handler() {
    /* TODO: Make this more effecient */
    sched_processes();
    do_task_switch();
}

static hal_timer_dev_t      _timer;
static timer_sp804_handle_t _sp804;
int hw_init_timer(uint32_t rate) {
    timer_sp804_init(&_sp804, VEXPRESS_A9_PERIPH_TIMER01_BASE);
    timer_sp804_int_attach(&_sp804, &intctlr, VEXPRESSA9_INT_TIM01);
    timer_sp804_create_timerdev(&_sp804, &_timer);
    /* Task switch timer: */
    hal_timer_dev_setfreq(&_timer, 0, rate);
    hal_timer_dev_attach(&_timer, 0, _task_switch_handler);
    /* Kernel time timer: */
    hal_timer_dev_setfreq(&_timer, 1, 100);
    hal_timer_dev_attach(&_timer, 1, _clk_count);

    return 0;
}
