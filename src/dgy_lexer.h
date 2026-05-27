#ifndef _dgy_lexer_h
#define _dgy_lexer_h

#include "dgy_all.h"

#define ERR_UNCLOSED_SYMBLE(sym) \
        L"\nError: Unclosed symble: '" sym "'\n"
#define ERR_INVALID_SYMBLE(sym) \
        L"\nError: Invalid symble: '" sym "'\n"
#define ERR_EXPECT_SYMBLE(sym) \
        L"\nError: Expect symble: '" sym "'\n"

typedef enum
{
        IMMD_DEC, /* Decimal immediate number */
        IMMD_HEX, /* Hexadecimal immediate number */
        STR,      /* String */
        CHAR,     /* Character */
        COMMENT
} SymbleType;

typedef struct
{
        wchar_t *symble;
        int type;
} SymbleTable;

typedef enum
{
        // Must be the same order of ReservedSymTable.type
        JIAN_CE_TIAO_JIAN,
        CHONG_FU_ZHI_XING,
        JIE_GUO_CUN,
        BU_CHENG_LI,
        WU_TIAO_JIAN,
        ZHE_LI_SHI,
        JU_HAO,
        CHENG_LI,
        ZHI_DAO,
        FOU_ZE,
        RU_GUO,
        CUN,
        DAO,
        LING,
        QIU,
        QU,
        JIU,
        SHE,
        RESERVED_SYM_CNT,
} ReservedSymType;

static const SymbleTable ReservedSymTable[] = {
    // Must be sorted in descending order of length
    {L"检测条件", JIAN_CE_TIAO_JIAN},
    {L"重复执行", CHONG_FU_ZHI_XING},
    {L"结果存", JIE_GUO_CUN},
    {L"不成立", BU_CHENG_LI},
    {L"无条件", WU_TIAO_JIAN},
    {L"这里是", ZHE_LI_SHI},
    {L"句号", JU_HAO},
    {L"成立", CHENG_LI},
    {L"直到", ZHI_DAO},
    {L"否则", FOU_ZE},
    {L"如果", RU_GUO},
    {L"存", CUN},
    {L"到", DAO},
    {L"令", LING},
    {L"求", QIU},
    {L"去", QU},
    {L"就", JIU},
    {L"设", SHE},
    {NULL, RESERVED_SYM_CNT},
};

typedef enum
{
        // Must be the same order of OpSymTable.type
        BEQ,
        AEQ,
        NEQ,
        BELOW,
        ABOVE,
        EQ,
        AND,
        OR,
        NOT,
        SLASH,
        HASH,
        AT,
        TILDE,

        OP_SYM_CNT,
} OpSymType;

static const SymbleTable OpSymTable[] = {
    // Must be sorted in descending order of length
    {L"<=", BEQ},
    {L">=", AEQ},
    {L"/=", NEQ},
    {L"<", BELOW},
    {L">", ABOVE},
    {L"=", EQ},
    {L"且", AND},
    {L"或", OR},
    {L"非", NOT},
    {L"/", SLASH},
    {L"#", HASH},
    {L"@", AT},
    {L"~", TILDE},
    {NULL, OP_SYM_CNT},
};

ErrCode fdgyDoLexer(const char *fname);

#endif /* _dgy_lexer_h */
