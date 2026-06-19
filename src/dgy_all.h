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
        CELL_FLAG_LEN,
  
        CELL_LEXER_IMMD,
        CELL_LEXER_STR,        
        CELL_LEXER_CHAR,
        CELL_LEXER_COMMENT,        
        CELL_LEXER_RESERVED,
        CELL_LEXER_OP,
        CELL_LEXER_WORD,
        CELL_LEXER_EXTERN_WORD,
        CELL_LEXER_WORD_CELL,
        CELL_LEXER_WORD_REG,
        CELL_LEXER_IMMD_CELL,
        CELL_LEXER_IMMD_REG,
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
