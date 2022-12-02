#ifndef ARCH_INTR_TYPES_PIC_H
#define ARCH_INTR_TYPES_PIC_H

#define PIC_IRQ_TIMER        ( 0)
#define PIC_IRQ_KEYBOARD     ( 1)
#define PIC_IRQ_SERIALA      ( 3)
#define PIC_IRQ_SERIALB      ( 4)
#define PIC_IRQ_PARALLELB    ( 5)
#define PIC_IRQ_FLOPPY       ( 6)
#define PIC_IRQ_PARALLELA    ( 7)
#define PIC_IRQ_RTC          ( 8)
#define PIC_IRQ_IRQ9         ( 9)
#define PIC_IRQ_IRQ10        (10)
#define PIC_IRQ_IRQ11        (11)
#define PIC_IRQ_PS2          (12)
#define PIC_IRQ_COPROCESSOR  (13)
#define PIC_IRQ_ATAPRIMARY   (14)
#define PIC_IRQ_ATASECONDARY (15)
#define PIC_IRQ_MAX          (16)

#define PIC_OFFSET_MASTER (32)
#define PIC_OFFSET_SLAVE  (PIC_OFFSET_MASTER + 8)

#define PIC_BIOS_OFFSET_MASTER (0x08)
#define PIC_BIOS_OFFSET_SLAVE  (0x70)

#define PIC1_BASE (0x20)
#define PIC2_BASE (0xa0)

#define PIC1_COMMAND (PIC1_BASE + 0)
#define PIC1_DATA    (PIC1_BASE + 1)
#define PIC2_COMMAND (PIC2_BASE + 0)
#define PIC2_DATA    (PIC2_BASE + 1)

#define PIC_ICW1_IC4  (1U << 0) /**< ICW4 needed */
#define PIC_ICW1_SNGL (1U << 1) /**< 1: Single mode, 0: Cascade mode */
#define PIC_ICW1_ADI  (1U << 2) /**< Call address interval (1: 4, 0: 8) */
#define PIC_ICW1_LTIM (1U << 3) /**< 1: Level triggered mode, 0: Edge triggered mode */
#define PIC_ICW1_1    (1U << 4) /**< Set to 1 */

#define PIC_ICW4_uPM  (1U << 0) /**< 0: MCS-80/85 Mode, 1: 8086/8088 Mode */
#define PIC_ICW4_AEOI (1U << 1) /**< Enable automatic EOI */
#define PIC_ICW4_MS   (1U << 2) /**< 0: Slave, 1: Master */
#define PIC_ICW4_BUF  (1U << 3) /**< Buffered mode enable */
#define PIC_ICW4_SFNM (1U << 4) /**< Enable Special Fully Nested Mode */

#define PIC_OCW2_EOI  (1U << 5) /**< End of interrupt */
#define PIC_OCW2_SL   (1U << 5) /**<  */
#define PIC_OCW2_R    (1U << 5) /**< Rotate */

#endif

