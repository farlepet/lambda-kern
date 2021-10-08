#ifndef ARCH_ACPI_ACPI_DATA_H
#define ARCH_ACPI_ACPI_DATA_H

#include "types.h"

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;

    /* The following are only valid for V2 */
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t  checksum_extended;
    uint8_t  _reserved[3];
} __packed acpi_rsdp_desc_t;

typedef struct {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __packed acpi_sdt_head_t;

typedef struct {
    acpi_sdt_head_t  head;
    acpi_sdt_head_t *entries[];
} __packed acpi_rsdt_t;

/** Multiple APIC Description Table */
typedef struct {
    acpi_sdt_head_t head;       /** Standard SDT header */
    uint32_t        lapic_addr; /** Local APIC address */
    uint32_t        flags;
    uint8_t         entries[];  /** MADT entries follow */
} __packed acpi_madt_t;

typedef struct {
    uint8_t type;   /** Entry type */
#define ACPI_MADT_ENTRYTYPE_LOCALAPIC     (0U) /** Local APIC */
#define ACPI_MADT_ENTRYTYPE_IOAPIC        (1U) /** I/O APIC */
#define ACPI_MADT_ENTRYTYPE_IOAPICSRCOVER (2U) /** I/O APIC Interrupt Source Override */
#define ACPI_MADT_ENTRYTYPE_IOAPICNMISRC  (3U) /**I/O APIC Non-maskable Interrupt Source */
#define ACPI_MADT_ENTRYTYPE_LAPICNMIS     (4U) /** Local APIC Non-maskable Interrupts */
#define ACPI_MADT_ENTRYTYPE_LAPICADDROVER (5U) /** Local APIC Address Override */
#define ACPI_MADT_ENTRYTYPE_LOCALX2APIC   (9U) /** Processor's Local x2APIC */
    uint8_t length; /** Entry length */
    uint8_t data[]; /** Entry contents */
} __packed acpi_madt_entry_t;

typedef struct {
    uint8_t  processor_id;
    uint8_t  apic_id;
    uint32_t flags;
#define ACPI_MADT_LAPIC_FLAG_ENABLED    (1UL << 0)
#define ACPI_MADT_LAPIC_FLAG_ONLINEABLE (1UL << 1)
} __packed acpi_madt_entry_lapic_t;

typedef struct {
    uint8_t  apic_id;
    uint8_t  _reserved;
    uint32_t apic_addr;
    uint32_t gsi_base;
} __packed acpi_madt_entry_ioapic_t;

typedef struct {
    uint8_t  bus_src;
    uint8_t  irq_src;
    uint32_t gsi;
    uint16_t flags;
} __packed acpi_madt_entry_ioapic_srcover_t;

typedef struct {
    uint8_t  nmi_src;
    uint8_t  _reserved;
    uint16_t flags;
    uint32_t gsi;
} __packed acpi_madt_entry_ioapic_nmisrc_t;

typedef struct {
    uint8_t  processor_id;
    uint16_t flags;
    uint8_t  int_n;
} __packed acpi_madt_entry_lapic_nmis_t;

typedef struct {
    uint16_t _reserved;
    uint64_t lapic_addr;
} __packed acpi_madt_entry_lapic_addrover_t;

typedef struct {
    uint16_t _reserved;
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t acpi_id;
} __packed acpi_madt_entry_proc_x2apic_t;

#endif
