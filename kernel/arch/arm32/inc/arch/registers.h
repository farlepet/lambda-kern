#ifndef ARCH_ARM32_REGISTERS_H
#define ARCH_ARM32_REGISTERS_H

#include <types.h>

#define __mrc(VAR, NAME, OPCODE, SRC, CREG, OPCODE2) asm volatile("mrc " #NAME ", " #OPCODE ", %0, " #SRC ", " #CREG ", " #OPCODE2 "\n" : "=r"(VAR))
#define __mcr(VAR, NAME, OPCODE, SRC, CREG, OPCODE2) asm volatile("mcr " #NAME ", " #OPCODE ", %0, " #SRC ", " #CREG ", " #OPCODE2 "\n" :: "r"(VAR))

#define __mrrc(VARL, VARH, NAME, OPCODE, CREG) asm volatile("mrrc " #NAME ", " #OPCODE ", %0, %1, " #CREG "\n" : "=r"(VARL), "=r"(VARH))
#define __mcrr(VARL, VARH, NAME, OPCODE, CREG) asm volatile("mcrr " #NAME ", " #OPCODE ", %0, %1, " #CREG "\n" :: "r"(VARL), "r"(VARH))

/* SCTLR - System Control Register */
#define __READ_SCTLR(VAR)  __mrc(VAR, p15, 0, c1, c0, 0)
#define __WRITE_SCTLR(VAR) __mcr(VAR, p15, 0, c1, c0, 0)

/* SCTLR - Secure Configuration Register */
#define __READ_SCR(VAR)  __mrc(VAR, p15, 0, c1, c1, 0)
#define __WRITE_SCR(VAR) __mcr(VAR, p15, 0, c1, c1, 0)

/* PERIPHBASE - Peripheral base address */
#define __READ_PERIPHBASE(VAR)  __mrc(VAR, p15, 4, c15, c0, 0)

/* VBAR - Vector Base Address Register */
#define __READ_VBAR(VAR)  __mrc(VAR, p15, 0, c12, c0, 0)
#define __WRITE_VBAR(VAR) __mcr(VAR, p15, 0, c12, c0, 0)

#endif
