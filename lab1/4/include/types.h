#ifndef _JOS_TYPES_H_
#define _JOS_TYPES_H_

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef char      int8_t;
typedef short     int16_t;
typedef int       int32_t;
typedef long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef uint32_t size_t;
typedef int32_t  ssize_t;

// Pointers and addresses are 32 bits long.
// We use pointer types to represent virtual addresses,
// uintptr_t to represent the numerical values of virtual addresses,
// and physaddr_t to represent physical addresses.
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef uint32_t physaddr_t;

// size_t is used for memory object sizes.
typedef uint32_t size_t;
// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef int32_t ssize_t;

// off_t is used for file offsets and lengths.
typedef int32_t off_t;

typedef char *va_list;

#define ROUNDDOWN(a,n)  ( (uint32_t)(a) / (uint32_t)(n) * (uint32_t)(n) )
#define ROUNDUP(a,n)    ( (((uint32_t)(a) - 1) / (uint32_t)(n) + 1) * (uint32_t)(n) )

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#endif /* _JOS_TYPES_H_ */