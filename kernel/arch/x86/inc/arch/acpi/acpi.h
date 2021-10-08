#ifndef ARCH_ACPI_ACPI_H
#define ARCH_ACPI_ACPI_H

#include <arch/acpi/acpi_data.h>
#include <arch/boot/multiboot.h>

void acpi_init(const mboot_t *head);

const acpi_sdt_head_t *acpi_find_sdt(const char *signature);

#endif
