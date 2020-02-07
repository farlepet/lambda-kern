#ifndef STDINT_H
#define STDINT_H

#ifdef ARCH_X86
typedef char      int8_t;  //!< Signed 8 bit value
typedef short     int16_t; //!< Signed 16 bit value
typedef int       int32_t; //!< Signed 32 bit value
typedef long long int64_t; //!< Signed 64 bit value

typedef unsigned char      uint8_t;  //!< Unsigned 8 bit value
typedef unsigned short     uint16_t; //!< Unsigned 16 bit value
typedef unsigned int       uint32_t; //!< Unsigned 32 bit value
typedef unsigned long long uint64_t; //!< Unsigned 64 bit value

typedef unsigned int ptr_t;  //!< Pointer

typedef unsigned int uintptr_t;  //!< Unsigned pointer
typedef int          intptr_t;   //!< Signed pointer

typedef unsigned int size_t; //!< Size/length
#endif

#endif