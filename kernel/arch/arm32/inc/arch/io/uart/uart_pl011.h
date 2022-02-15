#ifndef ARCH_ARM32_IO_UART_UART_PL011_H
#define ARCH_ARM32_IO_UART_UART_PL011_H

#include <stdint.h>

#include <arch/plat/platform.h>

#include <hal/io/char/char.h>
#include <hal/intr/int_ctlr.h>
#include <hal/clock/clock.h>
#include <io/input.h>

typedef struct {
    volatile uint32_t DR;    /** Data register */
#define UART_PL011_RSR_FE__POS    (     0) /** Framing error */
#define UART_PL011_RSR_PE__POS    (     1) /** Parity error */
#define UART_PL011_RSR_BE__POS    (     2) /** Break error */
#define UART_PL011_RSR_OE__POS    (     3) /** Overrun error */
    volatile uint32_t RSR;   /** Receive status & error clear */
    volatile uint32_t __reserved0[4];
#define UART_PL011_FR_CTS__POS    (     0) /** Clear to send */
#define UART_PL011_FR_DSR__POS    (     1) /** Data set ready */
#define UART_PL011_FR_DCD__POS    (     2) /** Data carrier detect */
#define UART_PL011_FR_BUSY__POS   (     3) /** Busy */
#define UART_PL011_FR_RXFE__POS   (     4) /** Receive FIFO empty */
#define UART_PL011_FR_TXFF__POS   (     5) /** Transmit FIFO full */
#define UART_PL011_FR_RXFF__POS   (     6) /** Receive FIFO full */
#define UART_PL011_FR_TXFE__POS   (     7) /** Transmit FIFO empty */
#define UART_PL011_FR_RI__POS     (     8) /** Ring indicator */
    volatile uint32_t FR;    /** Flags */
    volatile uint32_t __reserved1[1];
    volatile uint32_t ILPR;  /** IrDA low-power counter */
    volatile uint32_t IBRD;  /** Integer baud rate */
    volatile uint32_t FBRD;  /** Fractional baud rate (64x value after decimal) */
#define UART_PL011_LCR_BRK__POS   (     0) /** Send break */
#define UART_PL011_LCR_PEN__POS   (     1) /** Parity enable */
#define UART_PL011_LCR_EPS__POS   (     2) /** Even parity */
#define UART_PL011_LCR_STP2__POS  (     3) /** Two stop bits select*/
#define UART_PL011_LCR_FEN__POS   (     4) /** FIFO enable */
#define UART_PL011_LCR_WLEN__POS  (     5) /** Word length (5-8 bits) */
#define UART_PL011_LCR_WLEN__MASK (0x03UL)
#define UART_PL011_LCR_SPS__POS   (     7) /** Stick parity select */
    volatile uint32_t LCR;   /** Line control -- MUST be written to for divisor to take effect. */
#define UART_PL011_CR_UARTEN__POS (     0) /** UART enable */
#define UART_PL011_CR_SIREN__POS  (     1) /** SIR enable (IrDA) */
#define UART_PL011_CR_SIRLP__POS  (     2) /** SIR low power (IrDA) */
#define UART_PL011_CR_LBE__POS    (     7) /** Loopback enable */
#define UART_PL011_CR_TXE__POS    (     8) /** Transmit enable */
#define UART_PL011_CR_RXE__POS    (     9) /** Receive enable */
#define UART_PL011_CR_DTR__POS    (    10) /** Data transmit ready */
#define UART_PL011_CR_RTS__POS    (    11) /** Request to send */
#define UART_PL011_CR_OUT1__POS   (    12) /** UART out1 */
#define UART_PL011_CR_OUT2__POS   (    13) /** UART out2 */
#define UART_PL011_CR_RTSEN__POS  (    14) /** Requeset to send enable */
#define UART_PL011_CR_CTSEN__POS  (    15) /** Clear to send enable */
    volatile uint32_t CR;    /** Control */
#define UART_PL011_IFLS_TXIFLSEL__POS (    0) /** Transmit fifo interrupt level select */
#define UART_PL011_IFLS_RXIFLSEL__POS (    3) /** Receive fifo interrupt level select */
    volatile uint32_t IFLS;  /** Interrupt FIFO level select */
#define UART_PL011_IMSC_RIMIM__POS  (     0) /** Ring Indicator interrupt */
#define UART_PL011_IMSC_CTSMIM__POS (     1) /** Clear To Send interrupt */
#define UART_PL011_IMSC_DCDMIM__POS (     2) /** Data Carrier Detect interrupt */
#define UART_PL011_IMSC_DSRMIM__POS (     3) /** Data Set Ready interrupt */
#define UART_PL011_IMSC_RXIM__POS   (     4) /** Receive interrupt */
#define UART_PL011_IMSC_TXIM__POS   (     5) /** Transmit interrupt */
#define UART_PL011_IMSC_RTIM__POS   (     6) /** Receive Timeout interrupt */
#define UART_PL011_IMSC_FEIM__POS   (     7) /** Framing Error interrupt */
#define UART_PL011_IMSC_PEIM__POS   (     8) /** Parity Error interrupt */
#define UART_PL011_IMSC_BEIM__POS   (     9) /** Break Error interrupt */
#define UART_PL011_IMSC_OEIM__POS   (    10) /** Overrun Error interrupt */
    volatile uint32_t IMSC;  /** Interrupt mask set/clear */
    volatile uint32_t RIS;   /** Raw interrupt status */
    volatile uint32_t MIS;   /** Masked interrupt status */
    volatile uint32_t ICR;   /** Interrupt clear */
    volatile uint32_t DMACR; /** DMA control */
    volatile uint32_t __reserved2[13];
    volatile uint32_t __reserved3[4];
    volatile uint32_t __reserved4[976];
    volatile uint32_t __reserved5[4];
    volatile uint32_t PeriphID0;
    volatile uint32_t PeriphID1;
    volatile uint32_t PeriphID2;
    volatile uint32_t PeriphID3;
    volatile uint32_t CellID0;
    volatile uint32_t CellID1;
    volatile uint32_t CellID2;
    volatile uint32_t CellID3;
} uart_pl011_regmap_t;

typedef struct {
    uart_pl011_regmap_t *base;      /*!< Register map */
    hal_clock_dev_t     *src_clock; /*!< Clock source for UART */

    /* TODO: Should this be incorporated into hal_io_char_dev_t? */
    struct input_dev    *idev;
} uart_pl011_handle_t;

int uart_pl011_create_chardev(uart_pl011_handle_t *hand, hal_io_char_dev_t *chardev);

int uart_pl011_init(uart_pl011_handle_t *hand, void *base, hal_clock_dev_t *src_clock, uint32_t baud);

int uart_pl011_int_attach(uart_pl011_handle_t *hand, hal_intctlr_dev_t *intctlr, uint32_t int_n);

#endif
