/** \file gic.h
 *  \brief Contains support for the ARMv7 generic interrupt controller.
 *
 */
#ifndef ARCH_ARMV7_INTR_GIC_H
#define ARCH_ARMV7_INTR_GIC_H

#include <hal/intr/int_ctlr.h>

#include <arch/registers.h>

/** Snoop Control Unit registers */
typedef volatile struct {
    uint32_t SCUCTL;
    uint32_t SCUCONF;
    uint32_t SCUCPUPWRSTAT;
    uint32_t SCUINVALALL;
    uint32_t _reserved00[12];
    uint32_t FILTSTART;
    uint32_t FILTEND;
    uint32_t _reserved01[2];
    uint32_t SAC;
    uint32_t SNSAC;
    uint32_t _reserved02[42];
} armv7_scu_regmap_t;

/** Interrupt Interface registers */
typedef volatile struct {
#define ARMV7_GIC_ICC_ICCCTLR_GRP1ENABLE__POS        (       0)
#define ARMV7_GIC_ICC_ICCCTLR_GRP1FIQBYPDISABLE__POS (       5)
#define ARMV7_GIC_ICC_ICCCTLR_GRP1IRQBYPDISABLE__POS (       6)
#define ARMV7_GIC_ICC_ICCCTLR_EOIMODENS__POS         (       9)
    uint32_t ICCCTLR; /** CPU Interface Control */
#define ARMV7_GIC_ICC_ICCPMR_PRI__POS                (       0)
#define ARMV7_GIC_ICC_ICCPMR_PRI__MASK               (0x00FFUL)
    uint32_t ICCPMR;  /** Interrupt Priority Mask */
#define ARMV7_GIC_ICC_ICCBPR_BINPOINT__POS           (       0)
#define ARMV7_GIC_ICC_ICCBPR_BINPOINT__MASK          (0x0007UL)
    uint32_t ICCBPR;  /** Binary Point */
    uint32_t ICCIAR;  /** Interrupt Acknowledge */
    uint32_t ICCEOIR; /** End of Interrupt */
    uint32_t ICCRPR;  /** Running Priority */
    uint32_t ICCHPIR; /** Highest Pending Interrupt */
    uint32_t ICCABPR; /** Aliased Non-secure Binary Point */
    uint32_t _reserved00[55];
    uint32_t ICCIDR;  /** CPU Interface Implementor Identification */
} armv7_icc_regmap_t;

/** Global Timer registers */
typedef volatile struct {
    uint32_t CNT[2]; /** Counter */
    uint32_t CTRL;   /** Control */
    uint32_t ISTR;   /** Interrupt Status */
    uint32_t CMP[2]; /** Comparator Value */
    uint32_t AIR;    /** Auto-Increment */
    uint32_t _reserved00[57];
} armv7_gtm_regmap_t;

/** Private Timer and Watchdog registers */
typedef volatile struct {
    uint32_t PTIM_LOAD;   /** Private Timer Load */
    uint32_t PTIM_COUNT;  /** Private Timer Counter */
    uint32_t PTIM_CTRL;   /** Private Timer Control */
    uint32_t PTIM_ISTR;   /** Private Timer Interrupt Status */
    uint32_t _reserved00[4];
    uint32_t WDT_LOAD;    /** Watchdog Load */
    uint32_t WDT_COUNT;   /** Watchdog Counter */
    uint32_t WDT_CTRL;    /** Watchdog Control */
    uint32_t WDT_ISTR;    /** Watchdog Interrupt Status */
    uint32_t WDT_RSTST;   /** Watchdog Reset Status */
    uint32_t WDT_DISABLE; /** Watchdog Disable */
    uint32_t _reserved01[50];
} armv7_ptw_regmap_t;

/* Interrupt Distributor registers */
typedef volatile struct {
#define ARMV7_GIC_DCU_ICDDCR_SECUREEN__POS    (       0)
#define ARMV7_GIC_DCU_ICDDCR_NONSECUREEN__POS (       1)
    uint32_t ICDDCR;          /** Distributor Control */
    uint32_t ICDICTR;         /** Interrupt Controller Type */
    uint32_t ICDIIDR;         /** Distributor Implementor Identification */
    uint32_t _reserved00[29];
    uint32_t ICDISR[8];       /* Interrupt Security */
    uint32_t _reserved01[24];
    uint32_t ICDISER[8];      /* Interrupt Set-Enable */
    uint32_t _reserved02[24];
    uint32_t ICDICER[8];      /* Interrupt Clear-Enable */
    uint32_t _reserved03[24];
    uint32_t ICDISPR[32];     /* Interrupt Set-Pending */
    uint32_t ICDICPR[8];      /* Interrupt Clear-Pending */
    uint32_t _reserved04[24];
    uint32_t ICDABR[8];       /* Active Bit */
    uint32_t _reserved05[56];
    uint32_t ICDIPR[64];      /* Interrupt Priority */
    uint32_t _reserved06[192];
    uint32_t ICDIPTR[64];     /* Interrupt Processor Targets */
    uint32_t _reserved07[192];
    uint32_t ICDICFR[16];     /* Interrupt Configuration */
    uint32_t _reserved08[48];
    uint32_t ICPPISR;         /* PPI Status */
    uint32_t ICSPISR[7];      /* SPI Status */
    uint32_t _reserved09[120];
    uint32_t ICDSGIR;         /* Software Generated Interrupt */
    uint32_t _reserved10[51];
    uint32_t ICPIDR[8];       /* Peripheral IDs */
    uint32_t ICCIDR[4];       /* Component IDs */
} armv7_dcu_regmap_t;

/* TODO: Move this struct def */
typedef volatile struct {
    armv7_scu_regmap_t SCU;
    armv7_icc_regmap_t ICC;
    armv7_gtm_regmap_t GTM;
    uint32_t _reserved00[192];
    armv7_ptw_regmap_t PTW;
    uint32_t _reserved01[576];
    armv7_dcu_regmap_t DCU;
} armv7_mpcore_regmap_t;

typedef struct {
    uint32_t int_n;
    void (*callback)(uint32_t);
} armv7_gic_callback_t;

typedef struct {
    armv7_icc_regmap_t *icc;
    armv7_dcu_regmap_t *dcu;

#define ARMV7_GIC_MAX_CALLBACKS (16)
    armv7_gic_callback_t callbacks[ARMV7_GIC_MAX_CALLBACKS];    
} armv7_gic_handle_t;

int armv7_gic_create_intctlrdev(armv7_gic_handle_t *hand, hal_intctlr_dev_t *intctlrdev);

int armv7_gic_init(armv7_gic_handle_t *hand, volatile void *icc_base, volatile void *dcu_base);

int armv7_gic_irqhandle(armv7_gic_handle_t *hand);

#endif