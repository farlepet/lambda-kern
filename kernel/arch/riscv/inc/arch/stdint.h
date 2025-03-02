#ifndef ARCH_STDINT_H
#define ARCH_STDINT_H

typedef signed char int8_t;  //!< Signed 8 bit value
typedef short       int16_t; //!< Signed 16 bit value
typedef int         int32_t; //!< Signed 32 bit value

typedef unsigned char      uint8_t;  //!< Unsigned 8 bit value
typedef unsigned short     uint16_t; //!< Unsigned 16 bit value
typedef unsigned int       uint32_t; //!< Unsigned 32 bit value

#if defined(CONFIG_ARCH_CPU_RISCV_RV64I)
typedef long        int64_t; //!< Signed 64 bit value

typedef unsigned long  uint64_t; //!< Unsigned 64 bit value

typedef uint64_t ptr_t;  //!< Pointer

typedef uint64_t uintptr_t;  //!< Unsigned pointer
typedef int64_t  intptr_t;   //!< Signed pointer

typedef uint64_t size_t; //!< Size/length

/* @todo Move this elsewhere */
typedef uint64_t syscallarg_t; /** Syscall argument type */

#elif defined(CONFIG_ARCH_CPU_RISCV_RV32I) || \
      defined(CONFIG_ARCH_CPU_RISCV_RV32E)
typedef long long   int64_t; //!< Signed 64 bit value

typedef unsigned long long uint64_t; //!< Unsigned 64 bit value

typedef uint32_t ptr_t;  //!< Pointer

typedef uint32_t uintptr_t;  //!< Unsigned pointer
typedef int32_t intptr_t;   //!< Signed pointer

typedef uint32_t size_t; //!< Size/length

/* @todo Move this elsewhere */
typedef uint32_t syscallarg_t; /** Syscall argument type */

#endif /* CONFIG_ARCH_CPU_* */

#endif
