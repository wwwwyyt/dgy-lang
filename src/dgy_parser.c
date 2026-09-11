#include "dgy_parser.h"

static ErrCode getSymbol(FILE *in, DgyParser *parser);
static inline bool isReserved(SymbolType type, cell_t *s);
static inline bool isOp(SymbolType type, cell_t *s);
static inline bool isWord(cell_t *s);
static inline bool isExternWord(cell_t *s);
static inline bool isCellReg(cell_t *s);
static inline bool isStr(cell_t *s);
static inline bool isValue(cell_t *s);
static inline bool isRelationalOp(cell_t *s);
static inline bool isLogicalOp(cell_t *s);

static i32 match_WordBegin(DgyParser *parser, StatType *matchedType);
static i32 match_WordEnd(DgyParser *parser, StatType *matchedType);
static i32 match_Mov(DgyParser *parser, StatType *matchedType);
static i32 match_SimpWord(DgyParser *parser, StatType *matchedType);
static i32 match_Exec(DgyParser *parser, StatType *matchedType);
static i32 match_If(DgyParser *parser, StatType *matchedType);
static i32 match_Else(DgyParser *parser, StatType *matchedType);
static i32 match_ElseEnd(DgyParser *parser, StatType *matchedType);
static i32 match_Hereis(DgyParser *parser, StatType *matchedType);
static i32 match_Goto(DgyParser *parser, StatType *matchedType);
static i32 match_LoopBegin(DgyParser *parser, StatType *matchedType);
static i32 match_LoopCheck(DgyParser *parser, StatType *matchedType);
static i32 match_LoopEnd(DgyParser *parser, StatType *matchedType);

static bool matchStat(const StatType statType,
                      i32 (*match)(DgyParser *, StatType *),
                      DgyParser *parser, StatType *matchedType);

static const StatType _statType[] = {
    ST_WORD_BEGIN,
    ST_WORD_END,
    ST_MOV,
    ST_SIMP_WORD,
    ST_EXEC,
    ST_IF,
    ST_ELSE,
    ST_ELSE_END,
    ST_HEREIS,
    ST_GOTO,
    ST_LOOP_BEGIN,
    ST_LOOP_CHECK,
    ST_LOOP_END,
};

static i32 (*_matchStatFuncList[])(DgyParser *, StatType *) = {
    /* 必须与 statType[] 的元素顺序相同 */
    /* 例如 _matchStatFuncList[ST_WORD_BEGIN] 的值必须为 match_WordBegin */
    match_WordBegin, /* Word Declaration Begin 复杂词语声明 开始 */
    match_WordEnd,   /* Word Declaration End 复杂词语声明 结束 */
    match_Mov,       /* Move Value 存值 */
    match_SimpWord,  /* Simple Word Declaration 简单词语声明 */
    match_Exec,      /* Execute Word 词语执行 */
    match_If,        /* Branch Begin 条件分支 开始 */
    match_Else,      /* Branch Else 条件分支 否则 */
    match_ElseEnd,   /* Branch End  条件分支 结束 */
    match_Hereis,    /* Set Label 设置标记 */
    match_Goto,      /* Goto Label 跳转到标记 */
    match_LoopBegin, /* Loop Begin 循环开始 */
    match_LoopCheck, /* Loop Check 循环条件检查 */
    match_LoopEnd,   /* Loop End 循环结束 */
};

static ErrCode getSymbol(FILE *in, DgyParser *parser)
{
        DgyStack *analyStack = &(parser->analyStack),
                 *symbolStack = &(parser->symbolStack);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        if (CODE_SUCCESS == dgyDoLexerOnce(in, symbolStack))
        {
                cell_t top;
                dgyStackGetItemAt(symbolStack, -1, &top, _symbolIdx);
                if (CELL_FLAG_LEN == top.type)
                {
                        /* 多单元符号 */
                        /* cell = {
                            .data = (unused),
                            .type = CELL_LEXER_XXX
                        }
                        */
                        cell_t second;
                        /* 获取符号类型 */
                        dgyStackGetItemAt(symbolStack, -2, &second, _symbolIdx);
                        dgyStackPush(analyStack, second);
                }
                else
                {
                        /* 一单元符号 */
                        /* cell = {
                            .data = (symbol value),
                            .type = CELL_LEXER_XXX,
                        }
                        */
                        dgyStackPush(analyStack, top);
                }
                return CODE_SUCCESS;
        }
        else
        {
                return CODE_FAILURE;
        }
}

static inline bool isReserved(SymbolType type, cell_t *s)
{
        return s->type == CELL_LEXER_RESERVED &&
               s->data.sint == type;
}

static inline bool isOp(SymbolType type, cell_t *s)
{
        return s->type == CELL_LEXER_OP &&
               s->data.sint == type;
}

static inline bool isWord(cell_t *s)
{
        return s->type == CELL_LEXER_WORD;
}

static inline bool isExternWord(cell_t *s)
{
        return s->type == CELL_LEXER_EXTERN_WORD;
}

static inline bool isCellReg(cell_t *s)
{
        return s->type == CELL_LEXER_WORD_CELL ||
               s->type == CELL_LEXER_IMMD_CELL ||
               s->type == CELL_LEXER_WORD_REG ||
               s->type == CELL_LEXER_IMMD_REG;
}

static inline bool isStr(cell_t *s)
{
        return s->type == CELL_LEXER_STR;
}

static inline bool isValue(cell_t *s)
{
        return s->type == CELL_LEXER_WORD ||
               s->type == CELL_LEXER_IMMD ||
               s->type == CELL_LEXER_CHAR;
}

static inline bool isImmd(cell_t *s)
{
        return s->type == CELL_LEXER_IMMD;
}

static inline bool isRelationalOp(cell_t *s)
{
        if (s->type == CELL_LEXER_OP)
        {
                SymbolType type = s->data.sint;
                return type == S_BEQ ||
                       type == S_AEQ ||
                       type == S_NEQ ||
                       type == S_BELOW ||
                       type == S_ABOVE ||
                       type == S_EQ ||
                       type == S_AND ||
                       type == S_OR ||
                       type == S_NOT;
        }
        else
        {
                return false;
        }
}

static inline bool isLogicalOp(cell_t *s)
{
        if (s->type == CELL_LEXER_OP)
        {
                SymbolType type = s->data.sint;
                return type == S_AND ||
                       type == S_OR ||
                       type == S_NOT;
        }
        else
        {
                return false;
        }
}

static i32 match_WordBegin(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"词语声明开始";
        cell_t sym;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* <词语> */
                if (isWord(&sym))
                {
                        status = 1;
                        *matchedType = ST_WORD_BEGIN;
                        statement->data.WordBegin.word = *_symbolIdx;
                }
                break;
        case 1: /* "=" */
                if (isOp(S_EQ, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->type = ST_WORD_BEGIN;
                }
                else
                {
                        // 期望 "="
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_WordEnd(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"词语声明结束";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "=" */
                if (isOp(S_EQ, &sym))
                {
                        status = 1;
                        *matchedType = ST_WORD_END;
                }
                break;
        case 1: /* <词语> */
                if (isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.WordEnd.word = *_symbolIdx;
                        statement->type = ST_WORD_END;
                }
                else
                {
                        // 期望 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_Mov(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"存值语句";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "存" */
                if (isReserved(S_CUN, &sym))
                {
                        status = 1;
                        *matchedType = ST_MOV;
                }
                break;
        case 1: /* <数值> | <单元/寄存器> */
                if (isValue(&sym) || isCellReg(&sym))
                {
                        status = 2;
                        statement->data.Mov.src = *_symbolIdx;
                }
                else
                {
                        // 期望 <数值> 或 <单元/寄存器>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<数值> 或 <单元/寄存器>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* "到" */
                if (isReserved(S_DAO, &sym))
                {
                        status = 3;
                }
                else
                {
                        // 期望 "到"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"到"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 3: /* <单元> | <寄存器> */
                if (isCellReg(&sym) || isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.Mov.tar = *_symbolIdx;
                        statement->type = ST_MOV;
                }
                else
                {
                        // 期望 <单元/寄存器> 或 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<单元/寄存器> 或 <词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_SimpWord(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"简单词语声明语句";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "设" */
                if (isReserved(S_SHE, &sym))
                {
                        status = 1;
                        *matchedType = ST_SIMP_WORD;
                }
                break;
        case 1: /* <词语> */
                if (isWord(&sym))
                {
                        status = 2;
                        statement->data.SimpWord.word = *_symbolIdx;
                }
                else
                {
                        // 期望 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* "=" */
                if (isOp(S_EQ, &sym))
                {
                        status = 3;
                }
                else
                {
                        // 期望 "="
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 3: /* <单元/寄存器> | <数值> | <词语> | <外部词语> */
                if (isCellReg(&sym) ||
                    isValue(&sym) ||
                    isWord(&sym) ||
                    isExternWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.SimpWord.src = *_symbolIdx;
                        statement->type = ST_SIMP_WORD;
                }
                else
                {
                        // 期望 <单元/寄存器> | <数值> | <词语> | <外部词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<单元/寄存器> 或 <数值> 或 <词语> 或 <外部词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_Exec(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"执行语句";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "令" | "求" */
                if (isReserved(S_LING, &sym) ||
                    isReserved(S_QIU, &sym))
                {
                        status = 1;
                        *matchedType = ST_EXEC;
                }
                break;
        case 1: /* {<数值> | <外部词语> | <字符串> | <单元/寄存器>} ("结果存" | "无结果") */
                if (isValue(&sym) ||
                    isExternWord(&sym) ||
                    isStr(&sym) ||
                    isCellReg(&sym))
                {
                        status = 1;
                        statement->data.Exec.args[statement->data.Exec.i++] = *_symbolIdx;
                }
                else if (isReserved(S_JIE_GUO_CUN, &sym))
                {
                        status = 2;
                }
                else if (isReserved(S_WU_JIE_GUO, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.Exec.tar = -1; // -1 表示无结果
                        statement->type = ST_EXEC;
                }
                else
                {
                        // 期望 "结果存" 或 "无结果"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"'结果存' 或 '无结果'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <单元/寄存器> | <词语> */
                if (isCellReg(&sym) || isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.Exec.tar = *_symbolIdx;
                        statement->type = ST_EXEC;
                }
                else
                {
                        // 期望 <单元/寄存器> 或 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<单元/寄存器> 或 <词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_If(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"条件开始";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "如果" */
                if (isReserved(S_RU_GUO, &sym))
                {
                        status = 1;
                        *matchedType = ST_IF;
                }
                break;
        case 1: /* <数值> | <单元/寄存器> */
                if (isValue(&sym) || isCellReg(&sym))
                {
                        status = 2;
                        statement->data.If.cond[statement->data.If.i++] = *_symbolIdx;
                }
                else
                {
                        // 期望 <数值> 或 <单元/寄存器>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<数值> 或 <单元寄存器>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <关系操作符> | <逻辑操作符> | "就"*/
                if (isRelationalOp(&sym) || isLogicalOp(&sym))
                {
                        status = 1;
                        statement->data.If.cond[statement->data.If.i++] = *_symbolIdx;
                }
                else if (isReserved(S_JIU, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->type = ST_IF;
                }
                else
                {
                        // 期望 <关系操作符> 或 <逻辑操作符> 或 "就"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<关系运算符> 或 <逻辑运算符> 或 '就'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_Else(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "否则" */
                if (isReserved(S_FOU_ZE, &sym))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_ELSE;
                        statement->type = ST_ELSE;
                }
                break;
        }
        return status;
}

static i32 match_ElseEnd(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "否则结束" */
                if (isReserved(S_FOU_ZE_JIE_SHU, &sym))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_ELSE_END;
                        statement->type = ST_ELSE_END;
                }
                break;
        }
        return status;
}

static i32 match_Hereis(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"跳转标签声明语句";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "这里是" */
                if (isReserved(S_ZHE_LI_SHI, &sym))
                {
                        status = 1;
                        *matchedType = ST_HEREIS;
                }
                break;
        case 1: /* <名称> */
                if (isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.Hereis.label = *_symbolIdx;
                        statement->type = ST_HEREIS;
                }
                else
                {
                        // 期望 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_Goto(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"标签跳转语句";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "去" */
                if (isReserved(S_QU, &sym))
                {
                        status = 1;
                        *matchedType = ST_GOTO;
                }
                break;
        case 1: /* <名称> */
                if (isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.Goto.label = *_symbolIdx;
                        statement->type = ST_GOTO;
                }
                else
                {
                        // 期望 <词语>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_LoopBegin(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "重复执行" */
                if (isReserved(S_CHONG_FU_ZHI_XING, &sym))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_LOOP_BEGIN;
                        statement->type = ST_LOOP_BEGIN;
                }
                break;
        }
        return status;
}

static i32 match_LoopCheck(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"循环条件检测";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "检测" */
                if (isReserved(S_JIAN_CE, &sym))
                {
                        status = 1;
                        *matchedType = ST_LOOP_CHECK;
                }
                break;
        case 1: /* <数值> | <单元/寄存器> */
                if (isValue(&sym) || isCellReg(&sym))
                {
                        status = 2;
                        statement->data.LoopCheck.cond[statement->data.LoopCheck.i++] = *_symbolIdx;
                }
                else
                {
                        // 期望 <数值> 或 <单元/寄存器>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<数值> 或 <单元/寄存器>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <关系操作符> | <逻辑操作符> | "条件"*/
                if (isRelationalOp(&sym) || isLogicalOp(&sym))
                {
                        status = 1;
                        statement->data.LoopCheck.cond[statement->data.LoopCheck.i++] = *_symbolIdx;
                }
                else if (isReserved(S_TIAO_JIAN, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->type = ST_LOOP_CHECK;
                }
                else
                {
                        // 期望 <关系操作符> 或 <逻辑操作符> 或 "条件"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"<关系运算符> 或 <逻辑运算符> 或 '条件'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static i32 match_LoopEnd(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"循环结束";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStatement *statement = &(parser->statement);
        i32 *_symbolIdx = &(parser->_symbolIdx);
        cell_t sym;
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "直到" | "无条件" */
                if (isReserved(S_ZHI_DAO, &sym))
                {
                        status = 1;
                        *matchedType = ST_LOOP_END;
                }
                else if (isReserved(S_WU_TIAO_JIAN, &sym))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_LOOP_END;
                        statement->data.LoopEnd.type = 2;
                        statement->type = ST_LOOP_END;
                }
                break;
        case 1: /* "成立" */
                if (isReserved(S_CHENG_LI, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.LoopEnd.type = 1;
                        statement->type = ST_LOOP_END;
                }
                /* "不成立" */
                else if (isReserved(S_BU_CHENG_LI, &sym))
                {
                        status = MATCH_COMPLETED;
                        statement->data.LoopEnd.type = 0;
                        statement->type = ST_LOOP_END;
                }
                /* <立即数> | <词语> | <外部词语> */
                else if (isImmd(&sym) ||
                         isWord(&sym) ||
                         isExternWord(&sym))
                {
                        status = 2;
                        statement->data.LoopEnd.times = *_symbolIdx;
                        statement->type = ST_LOOP_END;
                }
                else
                {
                        // 期望 "成立" 或 "不成立" 或 <数值>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"'成立' 或 '不成立' 或 <数值>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* "次" */
                if (isReserved(S_CI, &sym))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // 期望 "次"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL(L"'次'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static bool matchStat(const StatType statType,
                      i32 (*match)(DgyParser *, StatType *),
                      DgyParser *parser, StatType *matchedType)
{
        bool matched = false;
        if (MATCH_COMPLETED == match(parser, matchedType))
        {
                // wprintf(L"归约 %u\n", *matchedType);
                matched = true;
                *matchedType = STATTYPE_UNDEFINED;
        }
        return matched;
}

ErrCode dgyDoParserOnce(DgyParser *parser, FILE *in)
{
        if (!parser || !in)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDoParser");
                return CODE_FAILURE;
        }
        StatType matchedType = STATTYPE_UNDEFINED;
        bool matched = false;
        // 清空符号栈
        dgyStackClear(&(parser->symbolStack));
        while (CODE_SUCCESS == getSymbol(in, parser))
        {
                if (matchedType == STATTYPE_UNDEFINED)
                {
                        for (i32 i = 0; i < STATTYPE_CNT; ++i)
                        {
                                matchStat(_statType[i],
                                          _matchStatFuncList[i],
                                          parser, &matchedType);
                                if (matchedType != STATTYPE_UNDEFINED)
                                {
                                        break;
                                }
                        }
                }
                else
                {
                        if (matchStat(matchedType,
                                      _matchStatFuncList[matchedType],
                                      parser, &matchedType))
                        {
                                matched = true;
                                break;
                        }
                }
        }
        if (matched == false)
        {
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}

ErrCode dgyParserInit(DgyParser *parser)
{
        if (!parser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyParserInit");
                return CODE_FAILURE;
        }
        memset(parser, 0, sizeof(DgyParser));
        /* 创建 分析栈 */
        dgyStackInit(&(parser->analyStack), 16);
        /* 创建 符号栈 */
        dgyStackInit(&(parser->symbolStack), 16);
        /* 初始化语句表示 */
        dgyStatInit(&(parser->statement));
        return CODE_SUCCESS;
}

ErrCode dgyParserDestroy(DgyParser *parser)
{
        if (!parser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyParserDestroy");
                return CODE_FAILURE;
        }
        /* 销毁 分析栈 */
        dgyStackDestroy(&(parser->analyStack));
        /* 销毁 符号栈 */
        dgyStackDestroy(&(parser->symbolStack));
        memset(parser, 0, sizeof(DgyParser));
        return CODE_SUCCESS;
}
