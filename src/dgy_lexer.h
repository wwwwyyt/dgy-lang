#ifndef _dgy_lexer_h
#define _dgy_lexer_h

#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"

typedef enum
{
        S_UNDEFINED = -1,

        /* Main type */
        S_IMMD = 0,
        S_STR,
        S_COMMENT,
        S_RESERVED,
        S_OP,
        S_WORD,
        S_CELL,
        SYM_CNT,

        /* Secondary type */
        S_CHAR, /* String which length == 1 */
        S_EXTERN_WORD,
        S_IMMD_CELL,        
        S_IMMD_REG,
        S_WORD_CELL,        
        S_WORD_REG,

        /* Reserved symbol type */
        S_RESERVED_UNDEFINED = -1,
        S_CHONG_FU_ZHI_XING = 0,
        S_FOU_ZE_JIE_SHU,
        S_JIE_GUO_CUN,
        S_WU_JIE_GUO,
        S_BU_CHENG_LI,
        S_WU_TIAO_JIAN,
        S_ZHE_LI_SHI,
        S_JIAN_CE,
        S_TIAO_JIAN,
        S_CHENG_LI,
        S_ZHI_DAO,
        S_FOU_ZE,
        S_RU_GUO,
        S_CUN,
        S_DAO,
        S_LING,
        S_QIU,
        S_QU,
        S_JIU,
        S_SHE,
        S_CI,
        RESERVED_SYM_CNT,

        /* Op symbol type */
        S_OP_UNDEFINED = -1,
        S_BEQ = 0,
        S_AEQ,
        S_NEQ,
        S_BELOW,
        S_ABOVE,
        S_EQ,
        S_AND,
        S_OR,
        S_NOT,
        S_SLASH,
        S_BACKSLASH,
        S_TILDE,
        OP_SYM_CNT,
} SymbolType;

enum
{
        MAX_IMMD_LEN = 20,
        MAX_STR_LEN = 1024,
        MAX_WORD_LEN = 1024,
};

ErrCode dgyDoLexerOnce(FILE *in, DgyStack *out);

#endif /* _dgy_lexer_h */
