/** \file types.h
 *  \brief Contains useful types and definitions.
 *
 *  Contains some types that are very useful for defining values of a
 *  certain size. Also contains many definitions to make the code much cleaner.
 */

#ifndef TYPES_H
#define TYPES_H

#include <config.h>

#ifdef ARCH_X86
typedef char      s8;  //!< Signed 8 bit value
typedef short     s16; //!< Signed 16 bit value
typedef int       s32; //!< Signed 32 bit value
typedef long long s64; //!< Signed 64 bit value

typedef unsigned char      u8;  //!< Unsigned 8 bit value
typedef unsigned short     u16; //!< Unsigned 16 bit value
typedef unsigned int       u32; //!< Unsigned 32 bit value
typedef unsigned long long u64; //!< Unsigned 64 bit value


typedef char      int8_t;  //!< Signed 8 bit value
typedef short     int16_t; //!< Signed 16 bit value
typedef int       int32_t; //!< Signed 32 bit value
typedef long long int64_t; //!< Signed 64 bit value

typedef unsigned char      uint8_t;  //!< Unsigned 8 bit value
typedef unsigned short     uint16_t; //!< Unsigned 16 bit value
typedef unsigned int       uint32_t; //!< Unsigned 32 bit value
typedef unsigned long long uint64_t; //!< Unsigned 64 bit value

typedef unsigned int ptr_t; //!< Pointer
#endif // ARCH_X86

typedef unsigned long long int max_ptr_t; //!< Maximum pointer size that can be used by an architecture


#define __alias(N)       __attribute__((__weak__, __alias__(#N))) //!< Create an alias to another object

#define __pure           __attribute__((__pure__))                //!< Return value depends soely on arguments

#define __align(A)       __attribute__((__aligned__(A)))         //!< Aligns the data A bytes
#define __inline_flat    __attribute__((__flatten__))             //!< Tries to inline ALL function calls

#define __error(E)       __attribute__((__error__(#E)))           //!< Throws an error is this is reached in preprocessing
#define __warning(E)     __attribute__((__warning__(#E)))         //!< Throws a warning is this is reached in preprocessing

#define __fastcall       __attribute__((__fastcall__))            //!< Causes arguments 1 & 2 to be passed in by ecx and edx
#define __thiscall       __attribute__((__thiscall__))            //!< Causes argument 1 to be passed in by ecx

#define __section(S)     __attribute__((__section__(#S)))         //!< Sets the section of the data
#define __noreturn       __attribute__((__noreturn__))            //!< The function doesn't return
#define __unused         __attribute__((__unused__))              //!< This is unused data
#define __packed         __attribute__((__packed__))              //!< Do not use alignment
#define __cold           __attribute__((__cold__))                //!< This data isn't used much
#define __hot            __attribute__((__hot__))                 //!< This data is used often

#define __vector_size(S) __attribute__ ((vector_size(S)))       //!< Create a vector

#ifndef ALLOW_DEPRECATED
#define __deprecated         __attribute__((__deprecated__))
#define __deprecated_msg(M)  __attribute__((__deprecated__(#M)))
#else
#define __deprecated
#define __deprecated_msg(M)
#endif

// Check if the compiler has certain built-ins
#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#define ARRAY_SZ(arry) (sizeof(arry) / sizeof(arry[0])) 

#define NULL (void *)0x00000000

typedef enum BOOL { FALSE, TRUE } bool;

void __builtin_ia32_pause(); //!< Energy-saving alternative to `nop`

typedef u32 (*func0_t)();
typedef u32 (*func1_t)(u32);
typedef u32 (*func2_t)(u32, u32);
typedef u32 (*func3_t)(u32, u32, u32);
typedef u32 (*func4_t)(u32, u32, u32, u32);
typedef u32 (*func5_t)(u32, u32, u32, u32, u32);

#endif
