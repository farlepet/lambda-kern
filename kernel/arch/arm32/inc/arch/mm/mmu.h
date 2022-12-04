#ifndef ARCH_MM_MMU_H
#define ARCH_MM_MMU_H

#include <types.h>

int armv7_mmu_init(void);

/* Short descriptor translation table entry formats */
#define MMU_DESCTYPE__MASK        (0x00000003) /*!< Descriptor type mask, not including supersection bit */
#define MMU_DESCTYPE_INVALID      (0x00000000) /*!< Invalid descriptor */
#define MMU_DESCTYPE_PAGETABLE    (0x00000001) /*!< Page table pointer - 4 KiB * 256 */
#define MMU_DESCTYPE_SECTION      (0x00000002) /*!< Section            - 1 MiB */
#define MMU_DESCTYPE_SUPERSECTION (0x00040002) /*!< Supersection       - 16 MiB */

/* Page Table descriptor */
#define MMU_DESC_PT_PXN__POS     (       2) /*!< Priveledged execute-never */
#define MMU_DESC_PT_NS__POS      (       3) /*!< Not-Secure */
#define MMU_DESC_PT_DOMAIN__POS  (       5) /*!< Domain */
#define MMU_DESC_PT_DOMAIN__MASK (     0xF)
#define MMU_DESC_PT_IMPL__POS    (       9) /*!< Implementation-defined */
#define MMU_DESC_PT_ADDR__POS    (      10) /*!< Page table address */
#define MMU_DESC_PT_ADDR__MASK   (0x3FFFFF)

/* Page Table entry types */
#define MMU_PTENTRYTYPE__MASK    (0x00000003)
#define MMU_PTENTRYTYPE_INVALID  (0x00000000)
#define MMU_PTENTRYTYPE_LARGE    (0x00000001)
#define MMU_PTENTRYTYPE_SMALL    (0x00000002)

/* Page Table entry fields */
#define MMU_PTENTRY_B__POS       (         2) /*!< Bufferable */
#define MMU_PTENTRY_C__POS       (         3) /*!< Cacheable */
#define MMU_PTENTRY_AP0__POS     (         4) /*!< Access Permissions 0 */
#define MMU_PTENTRY_AP1__POS     (         5) /*!< Access Permissions 1 */
#define MMU_PTENTRY_AP2__POS     (         9) /*!< Access Permissions 2 */
#define MMU_PTENTRY_S__POS       (        10) /*!< Shareable */
#define MMU_PTENTRY_NG__POS      (        11) /*!< Not-Global */

#define MMU_PTENTRY_LARGE_XN__POS      (        15) /*!< Execute-never */
#define MMU_PTENTRY_LARGE_TEX__POS     (        12) /*!< Memory region attribute bits */
#define MMU_PTENTRY_LARGE_TEX__MASK    (       0x7)
#define MMU_PTENTRY_LARGE_BASE__POS    (        16) /*!< Base address of memory region */
#define MMU_PTENTRY_LARGE_BASE__MASK   (    0xFFFF)

#define MMU_PTENTRY_SMALL_XN__POS      (         0) /*!< Execute-never */
#define MMU_PTENTRY_SMALL_TEX__POS     (         4) /*!< Memory region attribute bits */
#define MMU_PTENTRY_SMALL_TEX__MASK    (       0x7)
#define MMU_PTENTRY_SMALL_BASE__POS    (        12) /*!< Base address of memory region */
#define MMU_PTENTRY_SMALL_BASE__MASK   (   0xFFFFF)

/* Section descriptor fields */
#define MMU_DESC_S_PXN__POS            (         0) /*!< Priveledged execute-never */
#define MMU_DESC_S_B__POS              (         2) /*!< Bufferable */
#define MMU_DESC_S_C__POS              (         3) /*!< Cacheable */
#define MMU_DESC_S_XN__POS             (         4) /*!< Execute-never */
#define MMU_DESC_S_DOMAIN__POS         (         5) /*!< Domain */
#define MMU_DESC_S_DOMAIN__MASK        (       0xF)
#define MMU_DESC_S_IMPL__POS           (         9) /*!< Implementation-defined */
#define MMU_DESC_S_AP0__POS            (        10) /*!< Access Permissions 0 */
#define MMU_DESC_S_AP1__POS            (        11) /*!< Access Permissions 1 */
#define MMU_DESC_S_TEX__POS            (        12) /*!< Memory region attribute bits */
#define MMU_DESC_S_TEX__MASK           (       0x7)
#define MMU_DESC_S_AP2__POS            (        15) /*!< Access Permissions 2 */
#define MMU_DESC_S_S__POS              (        16) /*!< Sharable */
#define MMU_DESC_S_NG__POS             (        17) /*!< Not Global */
#define MMU_DESC_S_NS__POS             (        19) /*!< Not-secure */
#define MMU_DESC_S_BASE__POS           (        20) /*!< Base address of memory region */
#define MMU_DESC_S_BASE__MASK          (     0xFFF)

#endif
