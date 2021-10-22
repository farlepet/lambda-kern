/** \file gic.h
 *  \brief Contains support for the ARMv7 generic interrupt controller.
 *
 */
#ifndef ARCH_ARM32_INTR_GIC_H
#define ARCH_ARM32_INTR_GIC_H

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
#define ARM32_GIC_ICC_CTLR_GRP1ENABLE__POS        (       0)
#define ARM32_GIC_ICC_CTLR_GRP1FIQBYPDISABLE__POS (       5)
#define ARM32_GIC_ICC_CTLR_GRP1IRQBYPDISABLE__POS (       6)
#define ARM32_GIC_ICC_CTLR_EOIMODENS__POS         (       9)
    uint32_t CTLR;     /** CPU Interface Control */
#define ARM32_GIC_ICC_PMR_PRI__POS                (       0)
#define ARM32_GIC_ICC_PMR_PRI__MASK               (0x00FFUL)
    uint32_t PMR;      /** Interrupt Priority Mask */
#define ARM32_GIC_ICC_BPR_BINPOINT__POS           (       0)
#define ARM32_GIC_ICC_BPR_BINPOINT__MASK          (0x0007UL)
    uint32_t BPR;      /** Binary Point */
    uint32_t IAR;      /** Interrupt Acknowledge */
    uint32_t EOIR;     /** End of Interrupt */
    uint32_t RPR;      /** Running Priority */
    uint32_t HPIR;     /** Highest Pending Interrupt */
    uint32_t ABPR;     /** Aliased Non-secure Binary Point */
    uint32_t AIAR;     /** Aliased Interrupt Acknowledge */
    uint32_t AEOIR;    /** Aliased End of Interrupt */
    uint32_t AHPPIR;   /** Aliased Highest Priority Pending Interrupt */
    uint32_t STATUSR;  /** Error Reporting Status (optional) */
    uint32_t _reserved00[40];
    uint32_t APR[4];   /** Active Priorities */
    uint32_t NSAPR[4]; /** Non-Secure Active Priorities */
    uint32_t _reserved01[3];
    uint32_t IIDR;     /** CPU Interface Identification */
    uint32_t _reserved02[960];
    uint32_t DIR;      /** Deactivate Interrupt */
} armv7_gic_icc_regmap_t;

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
#define ARM32_GIC_DCU_CTLR_SECUREEN__POS    (       0)
#define ARM32_GIC_DCU_CTLR_NONSECUREEN__POS (       1)
    uint32_t CTLR;            /** Distributor Control */
    uint32_t TYPER;           /** Interrupt Controller Type */
    uint32_t IIDR;            /** Distributor Implementor Identification */
    uint32_t _reserved00[29];
    uint32_t IGROUPR[32];     /* Interrupt Security */
    uint32_t ISENABLER[32];   /* Interrupt Set-Enable */
    uint32_t ICENABLER[32];   /* Interrupt Clear-Enable */
    uint32_t ISPENDR[32];     /* Interrupt Set-Pending */
    uint32_t ICPENDR[32];     /* Interrupt Clear-Pending */
    uint32_t ISACTIVER[32];   /* Set Active Bit */
    uint32_t ICACTIVER[32];   /* Clear Active Bit */
    uint32_t IPRIORITYR[255]; /* Interrupt Priority */
    uint32_t _reserved01;
    uint32_t ITARGETSR[255];  /* Interrupt Processor Targets */
    uint32_t _reserved02;
    uint32_t ICFGR[64];
    uint32_t IGRPMODR[32];
    uint32_t _reserved03[32];
    uint32_t NSACR[64];
    uint32_t SGIR;            /* Software Generated Interrupt */
    /* There are more registers beyond this point as well. */
} armv7_gic_dcu_regmap_t;

/* TODO: Move this struct def */
typedef volatile struct {
    armv7_scu_regmap_t SCU;
    armv7_gic_icc_regmap_t ICC;
    armv7_gtm_regmap_t GTM;
    uint32_t _reserved00[192];
    armv7_ptw_regmap_t PTW;
    uint32_t _reserved01[576];
    armv7_gic_dcu_regmap_t DCU;
} armv7_mpcore_regmap_t;

typedef struct {
    uint32_t int_n;
    void *data;

    void (*callback)(uint32_t, void *);
} armv7_gic_callback_t;

typedef struct {
    armv7_gic_icc_regmap_t *icc;
    armv7_gic_dcu_regmap_t *dcu;

#define ARM32_GIC_MAX_CALLBACKS (16)
    armv7_gic_callback_t callbacks[ARM32_GIC_MAX_CALLBACKS];    
} armv7_gic_handle_t;

int armv7_gic_create_intctlrdev(armv7_gic_handle_t *hand, hal_intctlr_dev_t *intctlrdev);

int armv7_gic_init(armv7_gic_handle_t *hand, volatile void *icc_base, volatile void *dcu_base);

int armv7_gic_irqhandle(armv7_gic_handle_t *hand);

#endif