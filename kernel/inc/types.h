#ifndef TYPES_H
#define TYPES_H

typedef char      s8;
typedef short     s16;
typedef int       s32;
typedef long long s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define __alias(N)    __attribute__((__weak__, __alias__(#N))) // Create an alias to another object

#define __pure        __attribute__((__pure__))                // Return value depends soely on arguments

#define __align(A)    __attribute__((__aligned__(#A)))         // Aligns the data A bytes
#define __inline_flat __attribute__((__flatten__))             // Tries to inline ALL function calls

#define __error(E)    __attribute__((__error__(#E)))           // Throws an error is this is reached in preprocessing
#define __warning(E)  __attribute__((__warning__(#E)))         // Throws a warning is this is reached in preprocessing

#define __fastcall    __attribute__((__fastcall__))            // Causes arguments 1 & 2 to be passed in by ecx and edx
#define __thiscall    __attribute__((__thiscall__))            // Causes argument 1 to be passed in by ecx

#define __section(S)  __attribute__((__section__(#S)))         // Sets the section of the data
#define __noreturn    __attribute__((__noreturn__))            // The function doesn't return
#define __unused      __attribute__((__unused__))              // This is unused data
#define __packed      __attribute__((__packed__))              // Do not use alignment
#define __cold        __attribute__((__cold__))                // This data isn't used much
#define __hot         __attribute__((__hot__))                 // This data is used often

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

void __builtin_ia32_pause();

#endif