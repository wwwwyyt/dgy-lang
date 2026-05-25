#ifndef _dm_parser_h
#define _dm_parser_h

#include "dm_all.h"

#define ERR_UNCLOSED_SYMBLE(sym)                        \
        L"\nError: Unclosed symble: '" sym "'\n"
#define ERR_INVALID_SYMBLE(sym)                 \
        L"\nError: Invalid symble: '" sym "'\n"
#define ERR_EXPECT_SYMBLE(sym)                  \
        L"\nError: Expect symble: '" sym "'\n"

typedef enum
{
        IMMD_DEC,               /* Decimal immediate number */
        IMMD_HEX,               /* Hexadecimal immediate number */
        STR,                    /* String */
        CHAR,                   /* Character */
        COMMENT
} SymbleType;

static const wchar_t *ReservedSymTable[] = {
        L"存",
        L"到",
        L"令",
        L"求",
        L"去",
        L"就",
        L"如果",
        L"否则",
        L"直到",
        L"成立",
        L"句号",
        L"这里是",
        L"无条件",
        L"不成立",
        L"重复执行",
        L"检测条件",
        NULL
};

typedef enum
{
        CUN,
        DAO,
        LING,
        QIU,
        QU,
        JIU,
        RU_GUO,
        FOU_ZE,
        ZHI_DAO,
        CHENG_LI,
        JU_HAO,
        ZHE_LI_SHI,
        WU_TIAO_JIAN,
        BU_CHENG_LI,
        CHONG_FU_ZHI_XING,
        JIAN_CE_TIAO_JIAN
} ReservedSymType;

static const wchar_t *OpSymTable[] = {
        L"<",
        L">",
        L"=",
        L"且",
        L"或",
        L"非",
        L"/",
        L"#",
        L"@",
        L"~",
        L"<=",
        L">=",
        L"/=",
        NULL,
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
} OpSymType;

ErrCode fdmDoLexer(const char *fname);

#endif /* _dm_parser_h */
