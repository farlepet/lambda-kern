#ifndef ARCH_INTR_APIC_H
#define ARCH_INTR_APIC_H

#include <hal/intr/int_ctlr.h>

#include <types.h>

/** APIC register, with 32 of 128 bits being used. */
typedef struct {
    uint32_t value;
    uint32_t _reserved[3];
} __packed apic_reg_t;

typedef struct {
    apic_reg_t _reserved0[2];

    apic_reg_t id;
    apic_reg_t version;
    
    apic_reg_t _reserved1[4];
    
    apic_reg_t task_prio;
    apic_reg_t arbitration_prio;
    apic_reg_t processor_prio;
    
    apic_reg_t eoi;
    apic_reg_t remote_read;
    apic_reg_t logical_dest;
    apic_reg_t dest_format;
    apic_reg_t spurious_int;
    
    apic_reg_t in_service[8];
    apic_reg_t trig_mode[8];
    apic_reg_t int_request[8];
    
    apic_reg_t err_status;

    apic_reg_t _reserved2[6];

#define APIC_LVTENTRY_VECTOR__POS           (     0)
#define APIC_LVTENTRY_VECTOR__MASK          (0xFFUL)
#define APIC_LVTENTRY_DELIVERYMODE__POS     (     8)
#define APIC_LVTENTRY_DELIVERYMODE__MASK    (0x07UL)
#define APIC_LVTENTRY_DELIVERYMODE_FIXED    (   0UL)
#define APIC_LVTENTRY_DELIVERYMODE_LOWPRIO  (   1UL) /* Only valid for int_cmd */
#define APIC_LVTENTRY_DELIVERYMODE_SMI      (   2UL)
#define APIC_LVTENTRY_DELIVERYMODE_NMI      (   4UL)
#define APIC_LVTENTRY_DELIVERYMODE_EXTINT   (   5UL)
#define APIC_LVTENTRY_DELIVERYMODE_STARTUP  (   6UL) /* Only valid for int_cmd */
#define APIC_LVTENTRY_DELIVERYMODE_INIT     (   7UL)
#define APIC_LVTENTRY_DELIVERYSTATUS__POS   (    12)
#define APIC_LVTENTRY_INTRPOLARITY__POS     (    13)
#define APIC_LVTENTRY_REMOTEIRR__POS        (    14)
#define APIC_LVTENTRY_TRIGGERMODE__POS      (    15)
#define APIC_LVTENTRY_TRIGGERMODE_EDGE      (   0UL)
#define APIC_LVTENTRY_TRIGGERMODE_LEVEL     (   1UL)
#define APIC_LVTENTRY_MASKED__POS           (    16)
#define APIC_LVTENTRY_TIMERMODE__POS        (    17)
#define APIC_LVTENTRY_TIMERMODE__MASK       (0x03UL)
#define APIC_LVTENTRY_TIMERMODE_ONESHOT     (   0UL)
#define APIC_LVTENTRY_TIMERMODE_PERIODIC    (   1UL)
#define APIC_LVTENTRY_TIMERMODE_TSCDEADLINE (   2UL)
    apic_reg_t lvt_cmci;
    apic_reg_t int_cmd[2];
    apic_reg_t lvt_timer;
    apic_reg_t lvt_thermal;
    apic_reg_t lvt_performance;
    apic_reg_t lvt_lint[2];
    apic_reg_t lvt_error;
    
    apic_reg_t timer_initial_cnt;
    apic_reg_t timer_current_cnt;
    apic_reg_t _reserved3[4];
    apic_reg_t timer_div_config;
    apic_reg_t _reserved4;
} __packed apic_lapic_regs_t;

typedef struct {
    apic_lapic_regs_t *regs;
} apic_lapic_handle_t;

void apic_init(void);

uintptr_t apic_getaddr(void);

int apic_lapic_init(apic_lapic_handle_t *hand, uintptr_t base);

int apic_lapic_create_intctlrdev(apic_lapic_handle_t *hand, hal_intctlr_dev_t *intctlrdev);

#define MSR_APICBASE_BASE__POS  (                12)
#define MSR_APICBASE_BASE__MASK (0xFFFFFFFFFFFFFULL) /* Actual number of valid bits depends on hardware */
#define MSR_APICBASE_ENABLE     (1UL          << 11)
#define MSR_APICBASE_X2APIC     (1UL          << 10)
#define MSR_APICBASE_BSP        (1UL          <<  8)

#endif
