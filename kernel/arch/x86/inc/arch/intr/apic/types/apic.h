#ifndef ARCH_INTR_TYPES_APIC_H
#define ARCH_INTR_TYPES_APIC_H

#include <types.h>

/** APIC register, with 32 of 128 bits being used. */
typedef struct {
    volatile uint32_t value;
    uint32_t          _reserved[3];
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
#define LAPIC_SPURIOUS_VECTOR__POS          (     0) /**< Spurious interrupt vector */
#define LAPIC_SPURIOUS_VECTOR__MSK          (0xFFUL)
#define LAPIC_SPURIOUS_APICEN__POS          (     8) /**< APIC software enable */
#define LAPIC_SPURIOUS_DISFOCUSCHECK__POS   (     9) /**< Disable focus processor checking */
#define LAPIC_SPURIOUS_DISEOIBROADCAST__POS (    12) /**< Suppress EOI broadcasts */
    apic_reg_t spurious_int;
    
    apic_reg_t in_service[8];
    apic_reg_t trig_mode[8];
    apic_reg_t int_request[8];
    
    apic_reg_t err_status;

    apic_reg_t _reserved2[6];

#define LAPIC_LVTENTRY_VECTOR__POS           (     0)
#define LAPIC_LVTENTRY_VECTOR__MASK          (0xFFUL)
#define LAPIC_LVTENTRY_DELIVERYMODE__POS     (     8)
#define LAPIC_LVTENTRY_DELIVERYMODE__MASK    (0x07UL)
#define LAPIC_LVTENTRY_DELIVERYMODE_FIXED    (   0UL)
#define LAPIC_LVTENTRY_DELIVERYMODE_LOWPRIO  (   1UL) /* Only valid for int_cmd */
#define LAPIC_LVTENTRY_DELIVERYMODE_SMI      (   2UL)
#define LAPIC_LVTENTRY_DELIVERYMODE_NMI      (   4UL)
#define LAPIC_LVTENTRY_DELIVERYMODE_EXTINT   (   5UL)
#define LAPIC_LVTENTRY_DELIVERYMODE_STARTUP  (   6UL) /* Only valid for int_cmd */
#define LAPIC_LVTENTRY_DELIVERYMODE_INIT     (   7UL)
#define LAPIC_LVTENTRY_DELIVERYSTATUS__POS   (    12)
#define LAPIC_LVTENTRY_INTRPOLARITY__POS     (    13)
#define LAPIC_LVTENTRY_REMOTEIRR__POS        (    14)
#define LAPIC_LVTENTRY_TRIGGERMODE__POS      (    15)
#define LAPIC_LVTENTRY_TRIGGERMODE_EDGE      (   0UL)
#define LAPIC_LVTENTRY_TRIGGERMODE_LEVEL     (   1UL)
#define LAPIC_LVTENTRY_MASKED__POS           (    16)
#define LAPIC_LVTENTRY_TIMERMODE__POS        (    17)
#define LAPIC_LVTENTRY_TIMERMODE__MASK       (0x03UL)
#define LAPIC_LVTENTRY_TIMERMODE_ONESHOT     (   0UL)
#define LAPIC_LVTENTRY_TIMERMODE_PERIODIC    (   1UL)
#define LAPIC_LVTENTRY_TIMERMODE_TSCDEADLINE (   2UL)
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

typedef struct {
#define IOAPIC_ADDR_ID           (0x00)
#define IOAPIC_ID_ID__POS          (  24) /**< IOAPIC ID */
#define IOAPIC_ID_ID__MSK          (0x0F)
#define IOAPIC_ADDR_VER          (0x01)
#define IOAPIC_VER_VER__POS        (   0) /**< IOAPIC version */
#define IOAPIC_VER_VER__MSK        (0xFF)
#define IOAPIC_VER_MAXENT__POS     (  16) /**< Max redirection entry */
#define IOAPIC_VER_MAXENT__MSK     (0xFF)
#define IOAPIC_ADDR_ARBID        (0x02)
#define IOAPIC_ARBID_ARBID__POS    (  24) /**< Arbitration ID */
#define IOAPIC_ARBID_ARBID__MSK    (0x0F)
#define IOAPIC_ADDR_REDTBL_BASE  (0x10) /**< Redirection table base */
#define IOAPIC_REDTBL_INTVEC__POS  (   0)
#define IOAPIC_REDTBL_INTVEC__MSK  (0xFF)
#define IOAPIC_REDTBL_DELMOD__POS  (   8)
#define IOAPIC_REDTBL_DELMOD__MSK  (0x07)
#define IOAPIC_REDTBL_DESTMOD__POS (  11)
#define IOAPIC_REDTBL_DELIVS__POS  (  12)
#define IOAPIC_REDTBL_INTPOL__POS  (  13)
#define IOAPIC_REDTBL_REMIRR__POS  (  14)
#define IOAPIC_REDTBL_TRIDMOD__POS (  15)
#define IOAPIC_REDTBL_INTMSK__POS  (  16)
#define IOAPIC_REDTBL_DEST__POS    (  56)
#define IOAPIC_REDTBL_DEST__MSK    (0xFF)
    apic_reg_t ioregsel; /**< IO register select */
    apic_reg_t iowin;    /**< IO register window */
} __packed apic_ioapic_regs_t;

#endif

