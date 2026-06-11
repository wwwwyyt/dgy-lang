#ifndef _dgy_lexer_h
#define _dgy_lexer_h

#include "dgy_all.h"
#include "dgy_error.h"

typedef enum
{
        S_UNDEFINED = 0,
        S_IMMD,
        S_STR,
        S_CHAR, /* String which length == 1 */
        S_COMMENT,
        S_RESERVED,
        S_OP,
        S_WORD,
        S_EXTERN_WORD,
        S_CELL,
        S_REG,
} SymbolType;

typedef struct
{
        const wchar_t *symbol;
        int type;
} SymbolTable;

typedef enum
{
        // Must be the same order of _reservedSymTable.type
        S_RESERVED_UNDEFINED = 0,
        S_CHONG_FU_ZHI_XING,
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
        RESERVED_SYM_END,
} ReservedSymType;

static const SymbolTable _reservedSymTable[] = {
    // Must be sorted in descending order of length
    {NULL, 0},
    {L"重复执行", S_CHONG_FU_ZHI_XING},
    {L"否则结束", S_FOU_ZE_JIE_SHU},
    {L"结果存", S_JIE_GUO_CUN},
    {L"无结果", S_WU_JIE_GUO},
    {L"不成立", S_BU_CHENG_LI},
    {L"无条件", S_WU_TIAO_JIAN},
    {L"这里是", S_ZHE_LI_SHI},
    {L"检测", S_JIAN_CE},
    {L"条件", S_TIAO_JIAN},
    {L"成立", S_CHENG_LI},
    {L"直到", S_ZHI_DAO},
    {L"否则", S_FOU_ZE},
    {L"如果", S_RU_GUO},
    {L"存", S_CUN},
    {L"到", S_DAO},
    {L"令", S_LING},
    {L"求", S_QIU},
    {L"去", S_QU},
    {L"就", S_JIU},
    {L"设", S_SHE},
    {L"次", S_CI},
    {NULL, RESERVED_SYM_END},
};

typedef enum
{
        // Must be the same order of _opSymTable.type
        S_OP_UNDEFINED = 0,
        S_BEQ,
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

        OP_SYM_END,
} OpSymType;

static const SymbolTable _opSymTable[] = {
    // Must be sorted in descending order of length
    {NULL, 0},
    {L"<=", S_BEQ},
    {L">=", S_AEQ},
    {L"/=", S_NEQ},
    {L"<", S_BELOW},
    {L">", S_ABOVE},
    {L"=", S_EQ},
    {L"且", S_AND},
    {L"或", S_OR},
    {L"非", S_NOT},
    {L"/", S_SLASH},
    {L"\\", S_BACKSLASH},
    {L"~", S_TILDE},
    {NULL, OP_SYM_END},
};

enum
{
        MAX_IMMD_LEN = 20,
        MAX_WORD_LEN = 1024,
};
enum
{
        MAX_STR_LEN = 1024,
};

ErrCode fdgyDoLexer(const char *fname, wchar_t *out);
ErrCode dgyDoLexer(FILE *in, wchar_t *out, const int maxMatchedCnt);
ErrCode dgyDoLexerOnce(FILE *in, wchar_t *out);

#endif /* _dgy_lexer_h */
