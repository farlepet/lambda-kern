/** \file types.h
 *  \brief Contains useful types and definitions.
 *
 *  Contains some types that are very useful for defining values of a
 *  certain size. Also contains many definitions to make the code much cleaner.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include <config.h>

typedef unsigned long long int max_ptr_t; //!< Maximum pointer size that can be used by an architecture


#define __alias(N)       __attribute__((__weak__, __alias__(#N))) //!< Create an alias to another object

#define __pure           __attribute__((__pure__))                //!< Return value depends soely on arguments

#define __align(A)       __attribute__((__aligned__(A)))          //!< Aligns the data A bytes
#define __inline_flat    __attribute__((__flatten__))             //!< Tries to inline ALL function calls
#define __no_inline      __attribute__ ((__noinline__))           //!< Prevents inlining of function

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

typedef uint32_t (*func0_t)();
typedef uint32_t (*func1_t)(uint32_t);
typedef uint32_t (*func2_t)(uint32_t, uint32_t);
typedef uint32_t (*func3_t)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*func4_t)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*func5_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#endif
