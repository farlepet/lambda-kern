#ifndef ARCH_ARMV7_REGISTERS_H
#define ARCH_ARMV7_REGISTERS_H

#include <types.h>

#define __mrc(VAR, NAME, OPCODE, SRC, CREG, OPCODE2)  asm volatile("mrc " #NAME ", " #OPCODE ", %0, " #SRC ", " #CREG ", " #OPCODE2 "\n" : "=r"(VAR))
#define __mcr(VAR, NAME, OPCODE, SRC, CREG, OPCODE2) asm volatile("mcr " #NAME ", " #OPCODE ", %0, " #SRC ", " #CREG ", " #OPCODE2 "\n" :: "r"(VAR))

#define __mrrc(VARL, VARH, NAME, OPCODE, CREG)  asm volatile("mrc " #NAME ", " #OPCODE ", %0, %1, " #CREG "\n" : "=r"(VARL), "=r"(VARH))
#define __mcrr(VARL, VARH, NAME, OPCODE, CREG) asm volatile("mcr " #NAME ", " #OPCODE ", %0, %1, " #CREG "\n" :: "r"(VARL), "r"(VARH))

/* SCTLR - System Control Register */
#define __READ_SCTLR(VAR)  __mrc(VAR, p15, 0, c1, c0, 0)
#define __WRITE_SCTLR(VAR) __mcr(VAR, p15, 0, c1, c0, 0)

#endif
