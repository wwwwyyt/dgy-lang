#ifndef _dgy_cell_h
#define _dgy_cell_h

#include "dgy_type.h"

typedef enum
{
        CELL_UNDEFINED = -1, /* 用于类型未确定的元素 */
        CELL_FLAG_LEN,       /* 用于在 symbol stack 中存储多单元符号长度 */

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

        CELL_CODE_BUILTIN_WORD,
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

#endif /* _dgy_cell_h */
