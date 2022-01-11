#include <arch/init/hw_init.h>
#include <arch/intr/idt.h>
#include <arch/intr/pit.h>
#include <arch/proc/tasking.h>

__hot
static void _task_switch_handler() {
	/* TODO: Determine from timer struct */
	time_update(10);
	/* TODO: Make this more effecient */
	sched_processes();
	do_task_switch();
}

static hal_timer_dev_t _timer;
int hw_init_timer(uint32_t rate) {
	pit_init(rate);
	pit_create_timerdev(&_timer);
    hal_timer_dev_setfreq(&_timer, 0, rate);
	hal_timer_dev_attach(&_timer, 0, _task_switch_handler);

    return 0;
}