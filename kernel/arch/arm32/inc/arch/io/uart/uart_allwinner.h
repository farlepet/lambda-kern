#ifndef ARCH_ARM32_IO_UART_UART_ALLWINNER_H
#define ARCH_ARM32_IO_UART_UART_ALLWINNER_H

#include <stdint.h>

#include <hal/io/char/char.h>

#define UART_ALLWINNER_V3S_UART0_BASE (0x01C28000)
#define UART_ALLWINNER_V3S_UART1_BASE (0x01C28400)
#define UART_ALLWINNER_V3S_UART2_BASE (0x01C28800)

typedef struct
{
    union {
        volatile uint32_t RBR; //!< Receive buffer
        volatile uint32_t THR; //!< Transmit holding
        volatile uint32_t DLL; //!< Divisor latch low
    };
    union {
        volatile uint32_t DLH; //!< Divisor latch high
#define UART_ALLWINNER_IER_ERBFI__POS ( 0) //!< Enable received data available interrupt
#define UART_ALLWINNER_IER_ETBEI__POS ( 1) //!< Enable transmit holding register empty interrupt
#define UART_ALLWINNER_IER_ELSI__POS  ( 2) //!< Enable receiver line status interrupt
#define UART_ALLWINNER_IER_EDSSI__POS ( 3) //!< Enable modem status interrupt
#define UART_ALLWINNER_IER_PTIME__POS ( 7) //!< Enable THRE interrupt
        volatile uint32_t IER; //!< Interrupt enable
    };
    union {
#define UART_ALLWINNER_IIR_IID__POS      (     0) //!< Interrupt ID (highest priority)
#define UART_ALLWINNER_IIR_IID__MASK     (0x0FUL)
#define UART_ALLWINNER_IIR_IID_MSTATUS   (0x00UL) //!< Modem status
#define UART_ALLWINNER_IIR_IID_NONE      (0x01UL) //!< No pending interrupt
#define UART_ALLWINNER_IIR_IID_THREMPTY  (0x02UL) //!< THR empty
#define UART_ALLWINNER_IIR_IID_DATAAVAIL (0x04UL) //!< Received data available
#define UART_ALLWINNER_IIR_IID_RSTATUS   (0x06UL) //!< Reveiver line status
#define UART_ALLWINNER_IIR_IID_BUSY      (0x07UL) //!< Busy detect
#define UART_ALLWINNER_IIR_IID_CHTIMEOUT (0x0CUL) //!< Character timeout
#define UART_ALLWINNER_IIR_FEFLAG__POS   (     6) //!< FIFOs enabled flag
#define UART_ALLWINNER_IIR_FEFLAG__MASK  (0x03UL)
        volatile uint32_t IIR; //!< Interrupt identity
#define UART_ALLWINNER_FCR_FIFOE__POS    (     0) //!< Enable FIFOs
#define UART_ALLWINNER_FCR_RFIFOR__POS   (     1) //!< Reset RX FIFO
#define UART_ALLWINNER_FCR_XFIFOR__POS   (     2) //!< Reset TX FIFO
#define UART_ALLWINNER_FCR_DMAM__POS     (     3) //!< DMA mode
#define UART_ALLWINNER_FCR_TFT__POS      (     4) //!< TX empty threshold
#define UART_ALLWINNER_FCR_TFT__MASK     (0x03UL)
#define UART_ALLWINNER_FCR_RT__POS       (     6) //!< RX fifo trigger theshold
#define UART_ALLWINNER_FCR_RT__MASK      (0x03UL)
        volatile uint32_t FCR; //!< FIFO control
    };
    
#define UART_ALLWINNER_LCR_DLS__POS      (     0) //!< Data length select (5-8 bits)
#define UART_ALLWINNER_LCR_DLS__MASK     (0x03UL)
#define UART_ALLWINNER_LCR_STOP__POS     (     2) //!< Stop bits (0: 1 bit, 1: 2 bits (1.5 if DLS is zero))
#define UART_ALLWINNER_LCR_PEN__POS      (     3) //!< Parity enable
#define UART_ALLWINNER_LCR_EPS__POS      (     4) //!< Even parity select (0: 0dd, 1: even, 2nd bit: reverse)
#define UART_ALLWINNER_LCR_EPS__MASK     (0x03UL)
#define UART_ALLWINNER_LCR_BC__POS       (     6) //!< Break control
#define UART_ALLWINNER_LCR_DLAB__POS     (     7) //!< Divisor latch access enable (DLL and DLM)
    volatile uint32_t LCR;  //!< Line control
    volatile uint32_t MCR;  //!< Modem control
    volatile uint32_t LSR;  //!< Line status
    volatile uint32_t MSR;  //!< Moden status
    volatile uint32_t SCH;  //!< Scratch
    volatile uint32_t __reserved0[23];
    volatile uint32_t USR;  //!< Status
    volatile uint32_t TFL;  //!< Transmit FIFO level
    volatile uint32_t RFL;  //!< RFL
    volatile uint32_t __reserved1[7];
    volatile uint32_t HALT; //!< Halt TX
} uart_allwinner_regmap_t;

typedef struct {
    uart_allwinner_regmap_t *base;
} uart_allwinner_handle_t;

int uart_allwinner_create_chardev(uart_allwinner_handle_t *hand, hal_io_char_dev_t *chardev);

int uart_allwinner_init(uart_allwinner_handle_t *hand, void *base, uint32_t baud);

#endif