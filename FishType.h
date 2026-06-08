#pragma once

#include <stdint.h>
#include <stddef.h>
#include <Python.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef size_t      Size;
typedef void        u0;
typedef float       f32;
typedef double      f64;
typedef char*       Str;
typedef char        Word;
typedef long double f128;
typedef _Bool       Bool;
typedef void*       Ptr;
typedef u8*         BytesPtr;
#ifdef __SIZEOF_INT128__
    typedef signed __int128   s128;
    typedef unsigned __int128 u128;
    #define SL4(x) ((s128)x)
    #define UL4(x) ((u128)x)
#endif

#define True 1
#define False 0


