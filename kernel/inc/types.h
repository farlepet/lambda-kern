/** \file types.h
 *  \brief Contains useful types and definitions.
 *
 *  Contains some types that are very useful for defining values of a
 *  certain size. Also contains many definitions to make the code much cleaner.
 */

#ifndef TYPES_H
#define TYPES_H

typedef char      s8;  //!< Signed 8 bit value
typedef short     s16; //!< Signed 16 bit value
typedef int       s32; //!< Signed 32 bit value
typedef long long s64; //!< Signed 64 bit value

typedef unsigned char      u8;  //!< Unsigned 8 bit value
typedef unsigned short     u16; //!< Unsigned 16 bit value
typedef unsigned int       u32; //!< Unsigned 32 bit value
typedef unsigned long long u64; //!< Unsigned 64 bit value

#define __alias(N)    __attribute__((__weak__, __alias__(#N))) //!< Create an alias to another object

#define __pure        __attribute__((__pure__))                //!< Return value depends soely on arguments

#define __align(A)    __attribute__((__aligned__(#A)))         //!< Aligns the data A bytes
#define __inline_flat __attribute__((__flatten__))             //!< Tries to inline ALL function calls

#define __error(E)    __attribute__((__error__(#E)))           //!< Throws an error is this is reached in preprocessing
#define __warning(E)  __attribute__((__warning__(#E)))         //!< Throws a warning is this is reached in preprocessing

#define __fastcall    __attribute__((__fastcall__))            //!< Causes arguments 1 & 2 to be passed in by ecx and edx
#define __thiscall    __attribute__((__thiscall__))            //!< Causes argument 1 to be passed in by ecx

#define __section(S)  __attribute__((__section__(#S)))         //!< Sets the section of the data
#define __noreturn    __attribute__((__noreturn__))            //!< The function doesn't return
#define __unused      __attribute__((__unused__))              //!< This is unused data
#define __packed      __attribute__((__packed__))              //!< Do not use alignment
#define __cold        __attribute__((__cold__))                //!< This data isn't used much
#define __hot         __attribute__((__hot__))                 //!< This data is used often

#ifndef ALLOW_DEPRECATED
#define __deprecated         __attribute__((__deprecated__))
#define __deprecated_msg(M)  __attribute__((__deprecated__(#M)))
#else
#define __deprecated
#define __deprecated_msg(M)
#endif

#define __sti    asm volatile("sti")
#define __cli    asm volatile("cli")
#define __hlt    asm volatile("hlt")


#define NULL (void *)0x00000000

typedef enum BOOL { FALSE, TRUE } bool;

void __builtin_ia32_pause(); //!< Energy-saving alternative to `nop`

#endif