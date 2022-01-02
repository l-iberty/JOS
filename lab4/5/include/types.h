#ifndef JOS_INC_TYPES_H
#define JOS_INC_TYPES_H

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint32_t size_t;
typedef int32_t ssize_t;

typedef _Bool bool;
enum { false = 0, true = 1 };

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

// Efficient min and max operations
#define MIN(_a, _b)         \
  ({                        \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a <= __b ? __a : __b; \
  })
#define MAX(_a, _b)         \
  ({                        \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a >= __b ? __a : __b; \
  })

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)           \
  ({                              \
    uint32_t __a = (uint32_t)(a); \
    (typeof(a))(__a - __a % (n)); \
  })
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                     \
  ({                                                      \
    uint32_t __n = (uint32_t)(n);                         \
    (typeof(a))(ROUNDDOWN((uint32_t)(a) + __n - 1, __n)); \
  })

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#endif /* JOS_INC_TYPES_H */