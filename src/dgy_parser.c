#include "dgy_parser.h"
#include "dgy_analyser.h"
#include "dgy_error.h"
#include "dgy_stack.h"

static ErrCode getSymbol(FILE *in, DgyStack *symbolStack, DgyStack *analyStack);
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

static const StatType _statType[] =
{
        ST_WORD_BEGIN,
        ST_WORD_END,
        ST_MOV,
        ST_SET_REG,
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

static i32 (*_matchStatFuncList[])(DgyParser *, StatType *) =
{
        /* Must be the same order of statType[] */
        /* e.g. _matchStatFuncList[ST_WORD_BEGIN] = match_WordBegin */
        match_WordBegin,  /* Word Declaration Begin */
        match_WordEnd,    /* Word Declaration End */
        match_Mov,       /* Move Value */
        match_SimpWord,   /* Simple Word Declaration */
        match_Exec,      /* Execute Word */
        match_If,        /* Branch Begin */
        match_Else,      /* Branch Else */
        match_ElseEnd,   /* Branch End */
        match_Hereis,    /* Set Label */
        match_Goto,      /* Goto Label */
        match_LoopBegin, /* Loop Begin */
        match_LoopCheck, /* Loop Check */
        match_LoopEnd,   /* Loop End */
};

static ErrCode getSymbol(FILE *in, DgyStack *symbolStack, DgyStack *analyStack)
{        
        if (CODE_SUCCESS == dgyDoLexerOnce(in, symbolStack))
        {
                cell_t top;
                dgyStackTop(symbolStack, &top);
                if (CELL_FLAG_LEN == top.type)
                {
                        /* Multi-cell symbol */
                        /* cell =
                           { .data=(unused),
                             .type=CELL_LEXER_XXX }
                        */
                        cell_t second;
                        /* Get symbol's type */
                        dgyStackItemAt(symbolStack, 2, &second);
                        dgyStackPush(analyStack, second);
                }
                else
                {
                        /* Single-cell symbol */
                        /* cell =
                           { .data=(symbol value),
                             .type=CELL_LEXER_XXX }
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
        dgyStackTop(analyStack, &sym);
        dgyGetErr();
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* <Word> */
                if (isWord(&sym))
                {
                        status = 1;
                        *matchedType = ST_WORD_BEGIN;
                        dgyAppendData(analyser, symbolStack);
                }
                break;
        case 1: /* "=" */
                if (isOp(S_EQ, &sym))
                {
                        status = MATCH_COMPLETED;
                        dgyAppendInstr(analyser, ST_WORD_BEGIN);
                }
                else
                {
                        // Expect "="
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        case 1: /* <Word> */
                if (isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        dgyAppendData(analyser, symbolStack);
                        dgyAppendInstr(analyser, ST_WORD_END);
                }
                else
                {
                        // Expect <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        case 1: /* <Value> | <CellReg> */
                if (isValue(&sym) || isCellReg(&sym))
                {                        
                        status = 2;
                        dgyAppendData(analyser, symbolStack);
                }
                else
                {
                        // Expect <Value> or <CellReg>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<数值> 或 <单元/寄存器>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* "到" */
                if (isReserved(S_DAO, &sym))
                {
                        status = 3;
                }
                else
                {
                        // Expect "到"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("到"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 3: /* <CellReg> | <Word> */
                if (isCellReg(&sym) || isWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        dgyAppendAddr(analyser, symbolStack);
                        dgyAppendInstr(analyser, ST_MOV);
                }
                else
                {
                        // Expect <CellReg> or <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<单元/寄存器> 或 <词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
                        *matchedType = ST_SET_REG;
                }
                break;
        case 1: /* <Word> */
                if (isWord(&sym))
                {
                        status = 2;
                        dgyAppendData(analyser, symbolStack);
                }
                else
                {
                        // Expect <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* "=" */
                if (isOp(S_EQ, &sym))
                {
                        status = 3;
                }
                else
                {
                        // Expect "="
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 3: /* <CellReg> | <Value> | <Word> | <ExternWord> */
                if (isCellReg(&sym) ||
                    isValue(&sym) ||
                    isWord(&sym) ||
                    isExternWord(&sym))
                {
                        status = MATCH_COMPLETED;
                        dgyAppendData(analyser, symbolStack);
                        dgyAppendInstr(analyser, ST_MOV);
                }
                else
                {
                        // Expect <CellReg> | <Value> | <Word> | <ExternWord>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<单元/寄存器> 或 <数值> 或 <词语> 或 <外部词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        case 1: /* {<Value> | <ExternWord> | <Str> | <CellReg>} ("结果存" | "无结果") */
                if (isValue(&sym) ||
                    isExternWord(&sym) ||
                    isStr(&sym) ||
                    isCellReg(&sym))
                {
                        dgyAppendData(analyser, symbolStack);
                        status = 1;
                }
                else if (isReserved(S_JIE_GUO_CUN, &sym))
                {
                        status = 2;
                }
                else if (isReserved(S_WU_JIE_GUO, &sym))
                {
                        dgyAppendInstr(analyser, ST_EXEC);
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect "结果存" or "无结果"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("'结果存' or '无结果'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* <CellReg> | <Word> */
                if (isCellReg(&sym) || isWord(&sym))
                {
                        dgyAppendAddr(analyser, symbolStack);
                        dgyAppendInstr(analyser, ST_EXEC);
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <CellReg> or <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<单元/寄存器> 或 <词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        case 1: /* <Value> | <CellReg> */
                if (isValue(&sym) || isCellReg(&sym))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Value> or <CellReg>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<数值> 或 <单元寄存器>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* <Relational Op> | <Logical Op> | "就"*/
                if (isRelationalOp(&sym) || isLogicalOp(&sym))
                {
                        status = 1;
                }
                else if (isReserved(S_JIU, &sym))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Relational Op> or <Logical Op> or "就"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<关系运算符> 或 <逻辑运算符> 或 '就'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        }
        return status;
}

static i32 match_Else(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
                }
                break;
        }
        return status;
}

static i32 match_ElseEnd(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
                }
                else
                {
                        // Expect <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
                }
                else
                {
                        // Expect <Word>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<词语>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        }
        return status;
}

static i32 match_LoopBegin(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        DgyStack *analyStack = &(parser->analyStack);
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
        case 1: /* <Value> | <CellReg> */
                if (isValue(&sym) || isCellReg(&sym))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Value> or <CellReg>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<Value> or <CellReg>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* <Relational Op> | <Logical Op> | "条件"*/
                if (isRelationalOp(&sym) || isLogicalOp(&sym))
                {
                        status = 1;
                }
                else if (isReserved(S_TIAO_JIAN, &sym))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Relational Op> or <Logical Op> or "条件"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("<关系运算符> 或 <逻辑运算符> 或 '条件'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
                break;
        }
        return status;
}

static i32 match_LoopEnd(DgyParser *parser, StatType *matchedType)
{
        static i32 status = 0;
        static const wchar_t *statName = L"循环结束";
        DgyStack *analyStack = &(parser->analyStack);
        DgyStack *symbolStack = &(parser->symbolStack);
        DgyAnalyser *analyser = &(parser->analyser);
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
                }
                break;
        case 1: /* "成立" | "不成立" */
                if (isReserved(S_CHENG_LI, &sym) ||
                    isReserved(S_BU_CHENG_LI, &sym))
                {
                        status = MATCH_COMPLETED;
                }
                /* <Immd> | <Word> | <ExternWord> */
                else if (isImmd(&sym) ||
                         isWord(&sym) ||
                         isExternWord(&sym))
                {
                        status = 2;
                }
                else
                {
                        // Expect "成立" or "不成立" or <Value>
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("'成立' 或 '不成立' 或 <数值>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
                }
                break;
        case 2: /* "次" */
                if (isReserved(S_CI, &sym))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect "次"
                        wprintf(L"%ls: ", statName);
                        wprintf(ERR_EXPECT_SYMBOL("'次'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                        dgyDiscard(analyser);
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
                wprintf(L"归约 %u\n", *matchedType);
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
        DgyStack *analyStack = &(parser->analyStack),
                 *symbolStack = &(parser->symbolStack);
        StatType matchedType = STATTYPE_UNDEFINED;
        bool matched = false;
        while (CODE_SUCCESS == getSymbol(in, symbolStack, analyStack))
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

ErrCode dgyParserInit(DgyParser *parser,
                      DgyStack *codeStack,
                      DgyDict *wordDict)
{
        if (!parser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyParserInit");
                return CODE_FAILURE;
        }
        /* Create analyStack */
        dgyStackInit(&(parser->analyStack), 16);
        /* Create symbolStack */
        dgyStackInit(&(parser->symbolStack), 16);
        /* Create analyser */
        dgyAnalyserInit(&(parser->analyser), codeStack, wordDict);
        return CODE_SUCCESS;
}

ErrCode dgyParserDestroy(DgyParser *parser)
{
        if (!parser)
        {
                dgySetErr(ERR_NULLPTR, L"dgyParserDestroy");
                return CODE_FAILURE;
        }
        /* Destroy symbolStack */
        dgyStackDestroy(&(parser->analyStack));
        /* Destroy symbolStack */
        dgyStackDestroy(&(parser->symbolStack));
        memset(parser, 0, sizeof(DgyParser));
        return CODE_SUCCESS;
}
