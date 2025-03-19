#include <errno.h>

#include <arch/intr/pic.h>
#include <arch/io/ioport.h>

#include <lambda/export.h>

int pic_irq_disable(uint8_t irq_id) {
    if(irq_id >= 16) {
        return -EINVAL;
    }

    uint16_t port = PIC1_DATA;
    if(irq_id >= 8) {
        port = PIC2_DATA;
        irq_id -= 8;
    }

    uint8_t mask = inb(port) | (1U << irq_id);
    outb(port, mask);

    return 0;
}
EXPORT_FUNC(pic_irq_disable);

int pic_irq_enable(uint8_t irq_id) {
    if(irq_id >= 16) {
        return -EINVAL;
    }

    uint16_t port = PIC1_DATA;
    if(irq_id >= 8) {
        port = PIC2_DATA;
        irq_id -= 8;
    }

    uint8_t mask = inb(port) & ~(1U << irq_id);
    outb(port, mask);

    return 0;
}
EXPORT_FUNC(pic_irq_enable);

void pic_remap(uint8_t master, uint8_t slave) {
    /* Save interrupt masks */
    uint8_t m_mask = inb(PIC1_DATA);
    uint8_t s_mask = inb(PIC2_DATA);

    /* ICW 1 */
    outb(PIC1_COMMAND, PIC_ICW1_IC4 | PIC_ICW1_1);
    outb(PIC2_COMMAND, PIC_ICW1_IC4 | PIC_ICW1_1);
    /* ICW 2 - vector offset*/
    outb(PIC1_DATA, master);
    outb(PIC2_DATA, slave);
    /* ICW 3 */
    outb(PIC1_DATA, (1U << 2)); /* IRQ2 as slave input */
    outb(PIC2_DATA, 2);         /* Slave identity is 2 */
    /* ICW 4 */
    outb(PIC1_DATA, PIC_ICW4_uPM);
    outb(PIC2_DATA, PIC_ICW4_uPM);

    /* Restore interrupt masks */
    outb(PIC1_DATA, m_mask);
    outb(PIC2_DATA, s_mask);
}

