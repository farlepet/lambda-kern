#ifndef ARCH_PLAT_HW_BROADCOM_BCM2837_H
#define ARCH_PLAT_HW_BROADCOM_BCM2837_H

typedef struct {
    uint32_t ctrl;
    uint32_t _reserved_0;
    uint32_t core_timer_prescaler;
    uint32_t gpu_introuting;
    uint32_t perfmon_introute_set;
    uint32_t perfmon_introute_clr;
    uint32_t _reserved_1;
    uint32_t core_timer_access[2];
    uint32_t local_introute[2];
    uint32_t axi_outstanding_counters;
    uint32_t axi_outstanding_irq;
    uint32_t local_timer_ctrl;
    uint32_t local_timer_flags;
    uint32_t _reserved_2;
    uint32_t core_timer_intrctrl[4];
    uint32_t core_mailbox_intrctrl[4];
    uint32_t core_irqsource[4];
    uint32_t core_fiqsource[4];
    uint32_t core_mailbox_writeset[4][4];
    uint32_t core_mailbox_readclr[4][4];
} bcm2837_regs_t;

#define BROADCOM_BCM2837_REGS_BASE (void *)(0x40000000)

#define BROADCOM_BCM2837_PERIPHBASE_PI1 (void *)(0x20000000)
#define BROADCOM_BCM2837_PERIPHBASE_PI2 (void *)(0x3F000000)
#define BROADCOM_BCM2837_PERIPHBASE_PI4 (void *)(0xFE000000)

#define BROADCOM_BCM2837_PERIPHBASE_OFF_SYSTIMER (0x00003000UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_SP804    (0x0000B000UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_INTCTLR  (0x0000B200UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_TIMER    (0x0000B400UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_MAILBOX  (0x0000B880UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_GPIO     (0x00200000UL)
#define BROADCOM_BCM2837_PERIPHBASE_OFF_PL011    (0x00201000UL)


#endif /* ARCH_PLAT_HW_BROADCOM_BCM2837_H */
