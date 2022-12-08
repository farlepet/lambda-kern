#include <arch/init/hw_init.h>
#include <arch/io/uart/uart_pl011.h>
#include <arch/plat/platform.h>
#include <arch/intr/bcm2835_intctlr.h>
#include <arch/intr/timer/bcm2835_systimer.h>
#include <arch/io/bcm2835_gpio.h>
#include <arch/io/bcm2835_mailbox.h>
#include <arch/proc/tasking.h>
#include <arch/proc/stacktrace.h>

#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <io/output.h>

static uart_pl011_handle_t _pl011;
static hal_io_char_dev_t   _uart;

static bcm2835_intctlr_handle_t _bcm2835_intctlr;
hal_intctlr_dev_t               intctlr;

static uintptr_t _periphbase = (uintptr_t)BROADCOM_BCM2837_PERIPHBASE_PI2;

static hal_clock_dev_t _uartclock;

#if 0
__attribute__((aligned(16))) 
static volatile uint32_t _mbox_clkrate[12] = {
    sizeof(_mbox_clkrate), 0,
    0x38002, 12, 0, 2, 48000000,
    0,
    0, 0, 0, 0
};

__attribute__((aligned(16))) 
static volatile uint32_t _mbox_clken[12] = {
    sizeof(_mbox_clken), 0,
    0x30047, 4, 0, 2,
    0,
    0, 0, 0, 0, 0
};
#endif

int hw_init_console(void) {
    bcm2835_gpio_debug_init();
    bcm2835_gpio_debug(0x0);

    bcm2835_gpio_regmap_t *gpio =
        (bcm2835_gpio_regmap_t *)(_periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_GPIO);

    _uartclock.freq = 48000000;

#if 0
    /* Force to 3 MHz */
    bcm2835_mailbox_regmap_t *mailbox =
        (bcm2835_mailbox_regmap_t *)(_periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_MAILBOX);
    bcm2835_mailbox_write(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC, (uint32_t)&_mbox_clkrate);
    bcm2835_mailbox_read(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC);
    bcm2835_gpio_debug(0x1);
    
    bcm2835_mailbox_write(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC, (uint32_t)&_mbox_clken);
    bcm2835_mailbox_read(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC);
    //ioclock.freq = _mbox_clken[6];
    bcm2835_gpio_debug(0x2);
#endif

    /* Set UART pins to pulldown */
    bcm2835_gpio_setpull(gpio, 14, BCM2835_GPIO_PULL_PULLDOWN);
    bcm2835_gpio_setpull(gpio, 15, BCM2835_GPIO_PULL_PULLDOWN);
    /* Set UART pin functions to AUX0 */
    bcm2835_gpio_setfunc(gpio, 14, BCM2835_GPIO_FUNCTIONSELECT_ALT0);
    bcm2835_gpio_setfunc(gpio, 15, BCM2835_GPIO_FUNCTIONSELECT_ALT0);
    bcm2835_gpio_debug(0x3);
    
    uart_pl011_init(&_pl011,
                    (void *)(_periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_PL011),
                    &_uartclock,
                    115200);
    bcm2835_gpio_debug(0x4);
    uart_pl011_create_chardev(&_pl011, &_uart);
    output_set_dev(&_uart);

    bcm2835_gpio_debug(0x5);

    return 0;
}

int hw_init_interrupts(void) {
    kerror(ERR_INFO, "PERIPHBASE: %08X", _periphbase);

    bcm2835_gpio_debug(0x6);
    bcm2835_intctlr_init(&_bcm2835_intctlr,
                         (void *)(_periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_INTCTLR));
    bcm2835_intctlr_create_intctlrdev(&_bcm2835_intctlr, &intctlr);
    kerror(ERR_INFO, "Interrupt Controller Initialized");
    bcm2835_gpio_debug(0x7);

    uart_pl011_int_attach(&_pl011, &intctlr, BCM2835_IRQ_UART);
    bcm2835_gpio_debug(0x8);

    enable_interrupts();
    bcm2835_gpio_debug(0x9);

    return 0;
}

__hot
static void _clk_count() {
    /* TODO: Determine from timer struct */
    time_update(1000000);
}

__hot
static void _task_switch_handler() {
    /* TODO: Make this more effecient */
    sched_processes();
    do_task_switch();
}

static hal_timer_dev_t           _timer;
static bcm2835_systimer_handle_t _systimer;
int hw_init_timer(uint32_t rate) {
    bcm2835_systimer_init(&_systimer, (void *)(_periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_SYSTIMER));
    bcm2835_systimer_int_attach(&_systimer, &intctlr);
    bcm2835_systimer_create_timerdev(&_systimer, &_timer);
    /* Task switch timer: */
    hal_timer_dev_setfreq(&_timer, 0, rate);
    hal_timer_dev_attach(&_timer, 0, _task_switch_handler);
    /* Kernel time timer: */
    hal_timer_dev_setfreq(&_timer, 1, 1000);
    hal_timer_dev_attach(&_timer, 1, _clk_count);

    return 0;
}

/* TODO: Dynamically set base and size based on kernel size and memory capacity. */
#define ALLOC_BASE (0x00200000)
#define ALLOC_SIZE (0x00E00000)

int hw_init_mm(void) {
    /* TODO: Abstract this. Make memory map partially user-configurable. */
    init_alloc(ALLOC_BASE, ALLOC_SIZE);

    /* @note The peripheral address space extends beyond this, but this should
     * sufficient for now. */
    mmu_map(_periphbase, _periphbase, 0x00300000, MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL | MMU_FLAG_NOCACHE);

    return 0;
}

/* TODO: Move */
void arch_kpanic_hook(void) {
    stacktrace_here(16);

    kthread_t *thread = mtask_get_curr_thread();
    if(thread) {
        kprintf("\n--------- REGISTERS ---------\n");
        kprintf("r0:   xxxxxxxx r1:   %08x\n", thread->arch.gpregs.r1);
        kprintf("r2:   %08x r3:   %08x\n", thread->arch.gpregs.r2, thread->arch.gpregs.r3);
        kprintf("r4:   %08x r5:   %08x\n", thread->arch.gpregs.r4, thread->arch.gpregs.r5);
        kprintf("r6:   %08x r7:   %08x\n", thread->arch.gpregs.r6, thread->arch.gpregs.r7);
        kprintf("r8:   %08x r9:   %08x\n", thread->arch.gpregs.r8, thread->arch.gpregs.r9);
        kprintf("r10:  %08x r11:  %08x\n", thread->arch.gpregs.r10, thread->arch.gpregs.r11);
        kprintf("r12:  %08x\n", thread->arch.gpregs.r12);
        kprintf("ksp:  %08x usp:  %08x\n", thread->arch.regs.ksp, thread->arch.regs.usp);
        kprintf("klr:  %08x ulr:  %08x\n", thread->arch.regs.klr, thread->arch.regs.ulr);
        kprintf("cpsr: %08x spsr: %08x\n", thread->arch.regs.cpsr, thread->arch.regs.spsr);

        if(thread->arch.int_frame) {
            kprintf("--------- INTERRUPT ---------\n");
            kprintf("r0:   %08x r1:   %08x\n", thread->arch.int_frame->r[0],  thread->arch.int_frame->r[1]);
            kprintf("r2:   %08x r3:   %08x\n", thread->arch.int_frame->r[2],  thread->arch.int_frame->r[3]);
            kprintf("r4:   %08x r5:   %08x\n", thread->arch.int_frame->r[4],  thread->arch.int_frame->r[5]);
            kprintf("r6:   %08x r7:   %08x\n", thread->arch.int_frame->r[6],  thread->arch.int_frame->r[7]);
            kprintf("r8:   %08x r9:   %08x\n", thread->arch.int_frame->r[8],  thread->arch.int_frame->r[9]);
            kprintf("r10:  %08x r11:  %08x\n", thread->arch.int_frame->r[10], thread->arch.int_frame->r[11]);
            kprintf("r12:  %08x\n",            thread->arch.int_frame->r[12]);
            kprintf("lr:   %08x cpsr: %08x\n", thread->arch.int_frame->lr,    thread->arch.int_frame->cpsr);

            uint32_t lr_sys;
            asm volatile("cps #0x1F\n"
                         "mov %0, lr\n"
                         "cps #0x12\n" : "=r"(lr_sys));
            kprintf("lr_sys: %08x\n", lr_sys);

        }

        kprintf("-----------------------------\n");
    }

    bcm2835_gpio_debug_init();

    for(;;) {    
        bcm2835_gpio_debug(0xAA);
        _gpio_delay_cycles(100000000);
        bcm2835_gpio_debug(0x55);
        _gpio_delay_cycles(100000000);
    }
}
