#ifndef _dgy_lexer_h
#define _dgy_lexer_h

#include "dgy_all.h"

#define ERR_UNCLOSED_SYMBLE(sym) \
        L"\nError: Unclosed symble: '" sym "'\n"
#define ERR_INVALID_SYMBLE(sym) \
        L"\nError: Invalid symble: '" sym "'\n"
#define ERR_EXPECT_SYMBLE(sym) \
        L"\nError: Expect symble: '" sym "'\n"
#define WARN_OUT_OF_BUFFER(len) \
        L"\nWarning: Out of buffer. Exceeding length: %d\n", len

typedef enum
{
        S_UNDEFINED,
        S_IMMD,
        S_STR,
        S_COMMENT,
        S_RESERVED,
        S_OP,
        S_NAME,
} SymbleType;

typedef struct
{
        wchar_t *symble;
        int type;
} SymbleTable;

typedef enum
{
        // Must be the same order of ReservedSymTable.type
        S_JIAN_CE_TIAO_JIAN,
        S_CHONG_FU_ZHI_XING,
        S_FOU_ZE_JIE_SHU,
        S_JIE_GUO_CUN,
        S_BU_CHENG_LI,
        S_WU_TIAO_JIAN,
        S_ZHE_LI_SHI,
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
        RESERVED_SYM_CNT,
} ReservedSymType;

static const SymbleTable ReservedSymTable[] = {
    // Must be sorted in descending order of length
    {L"检测条件", S_JIAN_CE_TIAO_JIAN},
    {L"重复执行", S_CHONG_FU_ZHI_XING},
    {L"否则结束", S_FOU_ZE_JIE_SHU},
    {L"结果存", S_JIE_GUO_CUN},
    {L"不成立", S_BU_CHENG_LI},
    {L"无条件", S_WU_TIAO_JIAN},
    {L"这里是", S_ZHE_LI_SHI},
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
    {NULL, RESERVED_SYM_CNT},
};

typedef enum
{
        // Must be the same order of OpSymTable.type
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
        S_HASH,
        S_AT,
        S_TILDE,

        OP_SYM_CNT,
} OpSymType;

static const SymbleTable OpSymTable[] = {
    // Must be sorted in descending order of length
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
    {L"#", S_HASH},
    {L"@", S_AT},
    {L"~", S_TILDE},
    {NULL, OP_SYM_CNT},
};

enum
{
        MAX_IMMD_LEN = 20,
        MAX_NAME_LEN = 1024,
};
enum
{
        MAX_STR_LEN = 1024,
};

ErrCode fdgyDoLexer(const char *fname, wchar_t *out);
ErrCode dgyDoLexer(FILE *in, wchar_t *out, const int maxMatchedCnt);
ErrCode dgyDoLexerOnce(FILE *in, wchar_t *out);

#endif /* _dgy_lexer_h */
