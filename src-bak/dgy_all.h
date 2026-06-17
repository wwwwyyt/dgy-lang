#ifndef _dgy_all_h
#define _dgy_all_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>
#include <wctype.h>
#include <stdbool.h>

typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;

typedef enum
{
        T_UNDEFIND,
        T_UINT,
        T_SINT,
        T_REAL,
        T_WCHAR
} CellType;

typedef struct
{
        union
        {
                u64 uint;
                i64 sint;
                double real;
                wchar_t wchar;
        } data;
        CellType type;
} cell_t;

#endif /* _dgy_all_h */
