#ifndef _dm_parser_h
#define _dm_parser_h

#include "dm_all.h"

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
        CUN,
        DAO,
        LING,
        QIU,
        QU,
        JIU,
        SHE,
        RU_GUO,
        FOU_ZE,
        ZHI_DAO,
        CHENG_LI,
        JU_HAO,
        ZHE_LI_SHI,
        WU_TIAO_JIAN,
        BU_CHENG_LI,
        CHONG_FU_ZHI_XING,
        JIAN_CE_TIAO_JIAN,

        RESERVED_SYM_CNT,
} ReservedSymType;

static const SymbleTable ReservedSymTable[] = {
    {L"存", CUN},
    {L"到", DAO},
    {L"令", LING},
    {L"求", QIU},
    {L"去", QU},
    {L"就", JIU},
    {L"设", SHE},
    {L"如果", RU_GUO},
    {L"否则", FOU_ZE},
    {L"直到", ZHI_DAO},
    {L"成立", CHENG_LI},
    {L"句号", JU_HAO},
    {L"这里是", ZHE_LI_SHI},
    {L"无条件", WU_TIAO_JIAN},
    {L"不成立", BU_CHENG_LI},
    {L"重复执行", CHONG_FU_ZHI_XING},
    {L"检测条件", JIAN_CE_TIAO_JIAN},
    {NULL, RESERVED_SYM_CNT},
};

typedef enum
{
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
        BEQ,
        AEQ,
        NEQ,

        OP_SYM_CNT,
} OpSymType;

static const SymbleTable OpSymTable[] = {
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
    {L"<=", BEQ},
    {L">=", AEQ},
    {L"/=", NEQ},
    {NULL, OP_SYM_CNT},
};

ErrCode fdmDoLexer(const char *fname);

#endif /* _dm_parser_h */
