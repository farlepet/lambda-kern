#ifndef ARCH_INTR_TYPES_IDT_H
#define ARCH_INTR_TYPES_IDT_H

#include <stdint.h>

#include <intr/types/intr.h>

#pragma pack(push, 1)
/**
 * @brief IDT entry structure
 */
typedef struct x86_idt_entry_struct {
    uint16_t offset_low;  /**< Low 16 bits of offset */
    uint16_t segment;     /**< Segment selector */
    uint8_t  _reserved;   /**< Reserved bits, set to 0 */
    uint8_t  flags;       /**< IDT entry flags */
    uint16_t offset_high; /**< High 16 bits of offset */
} x86_idt_entry_t;

/**
 * @brief IDT Register (IDTR) structure
 */
typedef struct x8t_idt_idtr_struct {
    uint16_t limit;
    uint32_t base;
} x86_idt_idtr_t;
#pragma pack(pop)


/**
 * @brief IDT entry types
 */
enum x86_idt_ent_type_enum {
    IDT_ENTRY_TYPE_TASK32 = 0x05, /** 32-bit task gate */
    IDT_ENTRY_TYPE_INT16  = 0x06, /** 16-bit interrupt gate */
    IDT_ENTRY_TYPE_TRAP16 = 0x07, /** 16-bit trap gate */
    IDT_ENTRY_TYPE_INT32  = 0x0E, /** 32-bit interrupt gate */
    IDT_ENTRY_TYPE_TRAP32 = 0x0F  /** 32-bit trap gate */
};

/**
 * @brief Helper to populate IDT attribute field
 *
 * @param present is the entry present?
 * @param dpl privilege level
 * @param storeseg the GDT segment the handler is located in
 * @param type the type of interrupt this is
 * @see x86_idt_ent_type_enum
 */
#define IDT_ATTR(present, dpl, storeseg, type) \
    (((present & 0x01) << 7) | \
    ((dpl      & 0x03) << 5) |  \
    ((storeseg & 0x01) << 4) |   \
    ((type     & 0x0F) << 0))

/* @note It would be more efficient to have just an array of 256 items, but this
 * does not allow the flexibility of having multiple callbacks per interrupt.
 * This is currently used for timekeeping + scheduling. */

typedef struct {
    uint8_t  int_n;            /**< Interrupt ID this callback is associated with */
    uint8_t  _reserved[3];

    intr_handler_hand_t *hdlr; /**< Pointer to interrupt handler handle */
} x86_idt_callback_t;

typedef struct {
    uint8_t master_off; /**< Master PIC interrupt ID offset */
    uint8_t slave_off;  /**< Slave PIC interrupt ID offset */

#define X86_IDT_MAX (256)
    x86_idt_entry_t idt[X86_IDT_MAX]; /**< Interrupt descriptor table */
#define X86_IDT_MAX_CALLBACKS (16)
    x86_idt_callback_t callbacks[X86_IDT_MAX_CALLBACKS];
} x86_idt_handle_t;

#endif

