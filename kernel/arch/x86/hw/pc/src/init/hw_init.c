#include <arch/init/hw_init.h>
#include <arch/intr/idt.h>
#include <arch/intr/pit.h>
#include <arch/intr/apic/apictimer.h>
#include <arch/proc/tasking.h>

__hot
static void _task_switch_handler() {
    /* TODO: Determine from timer struct */
    time_update(10000000);
    /* TODO: Make this more effecient */
    sched_processes();
    do_task_switch();
}

static hal_timer_dev_t _timer;
#ifdef CONFIG_X86_APIC_TIMER
static apictimer_handle_t _apictimer_hand;
static hal_timer_dev_t _apictimer;
#endif

int hw_init_timer(uint32_t rate) {
    pit_init(rate);
    pit_create_timerdev(&_timer);
    hal_timer_dev_setfreq(&_timer, 0, rate);
#ifdef CONFIG_X86_APIC_TIMER
    apictimer_init(&_apictimer_hand);
    apictimer_create_timerdev(&_apictimer_hand, &_apictimer);
    hal_timer_dev_setfreq(&_apictimer, 0, rate);
    hal_timer_dev_attach(&_apictimer, 0, _task_switch_handler);
#else
    hal_timer_dev_attach(&_timer, 0, _task_switch_handler);
#endif

    return 0;
}
