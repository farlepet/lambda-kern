#include <arch/acpi/acpi.h>
#include <arch/boot/multiboot.h>
#include <arch/mm/paging.h>

#include <err/error.h>

#include <string.h>

static const acpi_rsdp_desc_t *_rsdp; /** Pointer to copy of RSDP structure */

static void _locate_rsdp(const mboot_t *head) {
#if (CONFIG_MULTIBOOT_VERSION == 2)
    const mboot_tag_acpi_rsdp_t *tag;
    if((tag = (const mboot_tag_acpi_rsdp_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_ACPI_NEW, 0))) {
        _rsdp = &tag->rsdp;
    } else if((tag = (const mboot_tag_acpi_rsdp_t *)multiboot_find_tag(head, MBOOT_TAGTYPE_ACPI_OLD, 0))) {
        _rsdp = &tag->rsdp;
    } else {
        _rsdp = NULL;
    }
#else
    /* Alternate methods not currently supported */
    (void)head;
    _rsdp = NULL;
#endif
}

static int _rsdp_checksum(const acpi_rsdp_desc_t *rsdp) {
    const uint8_t *rsdp_bytes = (const uint8_t *)rsdp;
    uint8_t        chksum     = 0;

    /* Validate v1 portion */
    for(size_t i = 0; i < 20; i++) {
        chksum += rsdp_bytes[i];
    }
    if(chksum) { return 0; }
    if(rsdp->revision == 0) { return 1; }
    
    /* Validate v2 portion */
    for(size_t i = 20; i < 36; i++) {
        chksum += rsdp_bytes[i];
    }
    if(chksum) { return 0; }
    return 1;
}

static int _sdt_checksum(const acpi_sdt_head_t *sdt) {
    const uint8_t *sdt_bytes = (const uint8_t *)sdt;
    uint8_t        chksum    = 0;

    for(uint32_t i = 0; i < sdt->length; i++) {
        chksum += sdt_bytes[i];
    }

    return chksum ? 0 : 1;
}

static void _print_rsdt(const acpi_rsdt_t *rsdt) {
    size_t n_sdt = (rsdt->head.length - sizeof(acpi_sdt_head_t)) / 4;
    kerror(ERR_DEBUG, "  RSDT: (%d entries)", n_sdt);
    for(size_t i = 0; i < n_sdt; i++) {
        if(!_sdt_checksum(rsdt->entries[i])) {
            kerror(ERR_DEBUG, "    [%d] ! Entry failed checksum validation!", i);
            continue;
        }
        kerror(ERR_DEBUG, "    [%d] %.4s | %.6s | %.8s", i,
                             rsdt->entries[i]->signature,
                             rsdt->entries[i]->oem_id,
                             rsdt->entries[i]->oem_table_id);
    }
}

void acpi_init(const mboot_t *head) {
    _locate_rsdp(head);

    if(!_rsdp) {
        kerror(ERR_DEBUG, "acpi: No RSDP structure reported by bootloader");
        return;
    }

    if(!_rsdp_checksum(_rsdp)) {
        kerror(ERR_DEBUG, "acpi: RSDP checksum validation failed!");
        return;
    }

    kerror(ERR_DEBUG, "acpi: RSDP:");
    kerror(ERR_DEBUG, "  signature: %.8s", _rsdp->signature);
    kerror(ERR_DEBUG, "  checksum:  %02X", _rsdp->checksum);
    kerror(ERR_DEBUG, "  OEM ID:    %.6s", _rsdp->oem_id);
    kerror(ERR_DEBUG, "  revision:  %02X", _rsdp->revision);
    kerror(ERR_DEBUG, "  RSDT addr: %08X", _rsdp->rsdt_address);
    kerror(ERR_DEBUG, "  XSDT addr: %08X", _rsdp->xsdt_addr);

    if(_rsdp->rsdt_address) {
        /* TODO: Map multiple pages, if necessary. Probably would be best to
         * just create a copy then free the pages, if we need this table after
         * initial bootup. */
        map_page((void *)_rsdp->rsdt_address, (void *)_rsdp->rsdt_address, 3);
        _print_rsdt((const acpi_rsdt_t *)_rsdp->rsdt_address);
    }
}

const acpi_sdt_head_t *acpi_find_sdt(const char *signature) {
    if(!_rsdp) {
        return NULL;
    }
    const acpi_rsdt_t *rsdt = (const acpi_rsdt_t *)_rsdp->rsdt_address;
    size_t n_sdt = (rsdt->head.length - sizeof(acpi_sdt_head_t)) / 4;
    for(size_t i = 0; i < n_sdt; i++) {
        if(!_sdt_checksum(rsdt->entries[i])) {
            continue;
        }
        if(!strncmp(rsdt->entries[i]->signature, signature, strlen(signature))) {
            return rsdt->entries[i];
        }
    }

    return NULL;
}
