#ifndef ARCH_PLAT_HW_BROADCOM_BCM2837_H
#define ARCH_PLAT_HW_BROADCOM_BCM2837_H

/* TODO: Either determine at runtime, or have this user-configurable. */
#define VEXPRESSA9_ALLOC_BASE (0x60200000)
#define VEXPRESSA9_ALLOC_SIZE (0x00E00000)

#define VEXPRESSA9_INT_WDOG0 (32)
#define VEXPRESSA9_INT_SWI   (33)
#define VEXPRESSA9_INT_TIM01 (34)
#define VEXPRESSA9_INT_TIM23 (35)
#define VEXPRESSA9_INT_RTC   (36)
#define VEXPRESSA9_INT_UART0 (37)
#define VEXPRESSA9_INT_UART1 (38)
#define VEXPRESSA9_INT_UART2 (39)
#define VEXPRESSA9_INT_UART3 (40)
#define VEXPRESSA9_INT_MCI0  (41)
#define VEXPRESSA9_INT_MCI1  (42)
#define VEXPRESSA9_INT_AACI  (43)
#define VEXPRESSA9_INT_KMI0  (44)
#define VEXPRESSA9_INT_KMI1  (45)
#define VEXPRESSA9_INT_CLCD  (46)
#define VEXPRESSA9_INT_ETH   (47)
#define VEXPRESSA9_INT_USB   (48)
#define VEXPRESSA9_INT_PCIE  (49)

#define VEXPRESS_A9_PERIPH_UART0_BASE   (void *)(0x10009000)
#define VEXPRESS_A9_PERIPH_UART1_BASE   (void *)(0x1000a000)
#define VEXPRESS_A9_PERIPH_UART2_BASE   (void *)(0x1000b000)
#define VEXPRESS_A9_PERIPH_UART3_BASE   (void *)(0x1000c000)

#define VEXPRESS_A9_PERIPH_TIMER01_BASE (void *)(0x10011000)
#define VEXPRESS_A9_PERIPH_TIMER23_BASE (void *)(0x10012000)

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

#endif /* ARCH_PLAT_HW_BROADCOM_BCM2837_H */
