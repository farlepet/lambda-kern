/** \file types.h
 *  \brief Contains useful types and definitions.
 *
 *  Contains some types that are very useful for defining values of a
 *  certain size. Also contains many definitions to make the code much cleaner.
 */

#ifndef TYPES_H
#define TYPES_H

#include "arch/proc/user.h"
#include <stdint.h>


#define __error(E)       __attribute__((__error__(#E)))           //!< Throws an error is this is reached in preprocessing
#define __warning(E)     __attribute__((__warning__(#E)))         //!< Throws a warning is this is reached in preprocessing

/* Function Attributes */
#define __pure           __attribute__((__pure__))                //!< Return value depends soely on arguments
#define __inline_flat    __attribute__((__flatten__))             //!< Tries to inline ALL function calls
#define __no_inline      __attribute__ ((__noinline__))           //!< Prevents inlining of function
#define __fastcall       __attribute__((__fastcall__))            //!< Causes arguments 1 & 2 to be passed in by ecx and edx
#define __thiscall       __attribute__((__thiscall__))            //!< Causes argument 1 to be passed in by ecx
#define __noreturn       __attribute__((__noreturn__))            //!< The function doesn't return
#define __naked          __attribute__((__naked__))               //!< Do not add callee instructions to this function
#define __weak           __attribute__((__weak__))                //!< Weak declaration

#ifdef __clang__
#  define __optimize_none  __attribute__((optnone))               //!< Prevent optimization of this function
#else
#  define __optimize_none  __attribute__((__optimize__("-O0")))   //!< Prevent optimization of this function
#endif
#ifndef ALLOW_DEPRECATED
#  define __deprecated         __attribute__((__deprecated__))
#  define __deprecated_msg(M)  __attribute__((__deprecated__(#M)))
#else
#  define __deprecated
#  define __deprecated_msg(M)
#endif

/* Type Attributes */
#define __align(A)       __attribute__((__aligned__(A)))          //!< Aligns the data A bytes
#define __packed         __attribute__((__packed__))              //!< Do not use alignment
#define __vector_size(S) __attribute__ ((vector_size(S)))       //!< Create a vector

/* General Attributes */
#ifndef __section
#  define __section(S)     __attribute__((__section__(#S)))       //!< Sets the section of the data
#endif

#define __alias(N)       __attribute__((__weak__, __alias__(#N))) //!< Create an alias to another object
#define __unused         __attribute__((__unused__))              //!< This is unused data
#define __used           __attribute__((__used__))                //!< Mark data/function as used
#define __cold           __attribute__((__cold__))                //!< This data isn't used much
#define __hot            __attribute__((__hot__))                 //!< This data is used often

/**
 * Used to wrap safety checks that are not strictly necessary for functionality.
 * Allows the disabling of these checks for potential performance, at the cost
 * of kernel safety/stability. Not to be used in a system that is not completely
 * locked down. */
#ifdef CONFIG_SAFETY_CHECKS
#  define SAFETY_CHECK(X) (X)
#else
#  define SAFETY_CHECK(X) (1)
#endif


// Check if the compiler has certain built-ins
#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#define ARRAY_SZ(arry) (sizeof(arry) / sizeof(arry[0])) 

#define __STR(X) #X
#define   STR(X) __STR(X)

void __builtin_ia32_pause(void); //!< Energy-saving alternative to `nop`

typedef uintptr_t syscallarg_t;

typedef syscallarg_t (*func0_t)(void);
typedef syscallarg_t (*func1_t)(syscallarg_t);
typedef syscallarg_t (*func2_t)(syscallarg_t, syscallarg_t);
typedef syscallarg_t (*func3_t)(syscallarg_t, syscallarg_t, syscallarg_t);
typedef syscallarg_t (*func4_t)(syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t);
typedef syscallarg_t (*func5_t)(syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t);
typedef syscallarg_t (*func6_t)(syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t, syscallarg_t);

/**
 * @brief Macro to try an operation, and return the result if it is < 0
 */
#define TRY_OR_RET(OP) ({ \
    int ret = (OP); \
    if (ret < 0) {  \
        return ret; \
    }               \
    ret;            \
})

#endif
