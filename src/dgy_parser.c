#include "dgy_parser.h"

static ErrCode getSymble(FILE *in, DgyStack *code, DgyStack *analysis);
static inline int isReserved(ReservedSymType type, cell_t *s, int top);
static inline int isOp(OpSymType type, cell_t *s, int top);
static inline int isName(cell_t *s, int top);
static inline int isCell(cell_t *s, int top);
static inline int isStr(cell_t *s, int top);
static inline int isValue(cell_t *s, int top);
static inline int isRelationalOp(cell_t *s, int top);
static inline int isLogicalOp(cell_t *s, int top);

static int match_ObjBegin(DgyStack *analysisStack, StatType *matchedType);
static int match_ObjEnd(DgyStack *analysisStack, StatType *matchedType);
static int match_Mov(DgyStack *analysisStack, StatType *matchedType);
static int match_SetReg(DgyStack *analysisStack, StatType *matchedType);
static int match_Exec(DgyStack *analysisStack, StatType *matchedType);
static int match_If(DgyStack *analysisStack, StatType *matchedType);
static int match_Else(DgyStack *analysisStack, StatType *matchedType);
static int match_ElseEnd(DgyStack *analysisStack, StatType *matchedType);
static int match_Hereis(DgyStack *analysisStack, StatType *matchedType);
static int match_Goto(DgyStack *analysisStack, StatType *matchedType);
static int match_LoopBegin(DgyStack *analysisStack, StatType *matchedType);
static int match_LoopCheck(DgyStack *analysisStack, StatType *matchedType);
static int match_LoopEnd(DgyStack *analysisStack, StatType *matchedType);

static ErrCode getSymble(FILE *in, DgyStack *codeStack, DgyStack *analysisStack)
{
        enum
        {
                MAX_BUF_SIZE = MAX_NAME_LEN + 2
        };
        static wchar_t buffer[MAX_BUF_SIZE];
        ErrCode code = dgyDoLexerOnce(in, buffer);
        if (code != CODE_SUCCESS)
                return code;
        for (int i = 0; i < MAX_BUF_SIZE - 1; ++i)
        {
                dgyStackPush(codeStack, buffer[i]);
                if (buffer[i + 1] == L'\0')
                {
                        /* Push symble type to this stack for analysis */
                        SymbleType type = buffer[i];
                        if (type == S_RESERVED || type == S_OP)
                        {
                                /* Push ReservedSymType or OpSymType */
                                dgyStackPush(analysisStack, buffer[i - 1]);
                        }
                        dgyStackPush(analysisStack, type);
                        break;
                }
        }
        return code;
}

static inline int isReserved(ReservedSymType type, cell_t *s, int top)
{
        return top >= 1 && s[top] == S_RESERVED && s[top - 1] == type;
}

static inline int isOp(OpSymType type, cell_t *s, int top)
{
        return top >= 1 && s[top] == S_OP && s[top - 1] == type;
}

static inline int isName(cell_t *s, int top)
{
        return top >= 0 && s[top] == S_NAME;
}

static inline int isCell(cell_t *s, int top)
{
        return top >= 0 && s[top] == S_CELL;
}

static inline int isStr(cell_t *s, int top)
{
        return top >= 0 && s[top] == S_STR;
}

static inline int isValue(cell_t *s, int top)
{
        if (top >= 0)
        {
                SymbleType type = s[top];
                return type == S_NAME || type == S_IMMD || type == S_CHAR;
        }
        else
        {
                return 0;
        }
}

static inline int isRelationalOp(cell_t *s, int top)
{
        if (top >= 1 && s[top] == S_OP)
        {
                OpSymType type = s[top - 1];
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
                return 0;
        }
}

static inline int isLogicalOp(cell_t *s, int top)
{
        if (top >= 1 && s[top] == S_OP)
        {
                OpSymType type = s[top - 1];
                return type == S_AND ||
                       type == S_OR ||
                       type == S_NOT;
        }
        else
        {
                return 0;
        }
}

static int match_ObjBegin(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* <Name> */
                if (isName(s, top))
                {
                        status = 1;
                        *matchedType = ST_OBJ_BEGIN;
                }
                break;
        case 1: /* "=" */
                if (isOp(S_EQ, s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect "="
                        wprintf(ERR_EXPECT_SYMBLE("="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_ObjEnd(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "=" */
                if (isOp(S_EQ, s, top))
                {
                        status = 1;
                        *matchedType = ST_OBJ_END;
                }
                break;
        case 1: /* <Name> */
                if (isName(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_Mov(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "存" */
                if (isReserved(S_CUN, s, top))
                {
                        status = 1;
                        *matchedType = ST_MOV;
                }
                break;
        case 1: /* <Value> | <Cell> */
                if (isValue(s, top) || isCell(s, top))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Value> or <Cell>
                        wprintf(ERR_EXPECT_SYMBLE("<Value> or <Cell>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* "到" */
                if (isReserved(S_DAO, s, top))
                {
                        status = 3;
                }
                else
                {
                        // Expect "到"
                        wprintf(ERR_EXPECT_SYMBLE("到"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 3: /* <Cell> | <Name> */
                if (isCell(s, top) || isName(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Cell> or <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Cell> or <Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_SetReg(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "设" */
                if (isReserved(S_SHE, s, top))
                {
                        status = 1;
                        *matchedType = ST_SET_REG;
                }
                break;
        case 1: /* <Name> */
                if (isName(s, top))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* "=" */
                if (isOp(S_EQ, s, top))
                {
                        status = 3;
                }
                else
                {
                        // Expect "="
                        wprintf(ERR_EXPECT_SYMBLE("="));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 3: /* <Cell> | <Value> */
                if (isCell(s, top) || isValue(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Cell> or <Value>
                        wprintf(ERR_EXPECT_SYMBLE("<Cell> or <Value>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_Exec(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "令" | "求" */
                if (isReserved(S_LING, s, top) ||
                    isReserved(S_QIU, s, top))
                {
                        status = 1;
                        *matchedType = ST_EXEC;
                }
                break;
        case 1: /* {<Value> | <Str>} "结果存" */
                if (isValue(s, top) || isStr(s, top))
                {
                        status = 1;
                }
                else if (isReserved(S_JIE_GUO_CUN, s, top))
                {
                        status = 2;
                }
                else
                {
                        // Expect "结果存"
                        wprintf(ERR_EXPECT_SYMBLE("结果存"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <Cell> | <Name> */
                if (isCell(s, top) || isName(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Cell> or <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Cell> or <Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_If(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "如果" */
                if (isReserved(S_RU_GUO, s, top))
                {
                        status = 1;
                        *matchedType = ST_IF;
                }
                break;
        case 1: /* <Value> | <Cell> */
                if (isValue(s, top) || isCell(s, top))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Value> or <Cell>
                        wprintf(ERR_EXPECT_SYMBLE("<Value> or <Cell>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <Relational Op> | <Logical Op> | "就"*/
                if (isRelationalOp(s, top) || isLogicalOp(s, top))
                {
                        status = 1;
                }
                else if (isReserved(S_JIU, s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Relational Op> or <Logical Op> or "就"
                        wprintf(ERR_EXPECT_SYMBLE("<Relational Op> or <Logical Op> or '就'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_Else(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "否则" */
                if (isReserved(S_FOU_ZE, s, top))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_ELSE;
                }
                break;
        }
        return status;
}

static int match_ElseEnd(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "否则结束" */
                if (isReserved(S_FOU_ZE_JIE_SHU, s, top))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_ELSE_END;
                }
                break;
        }
        return status;
}

static int match_Hereis(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "这里是" */
                if (isReserved(S_FOU_ZE_JIE_SHU, s, top))
                {
                        status = 1;
                        *matchedType = ST_HEREIS;
                }
                break;
        case 1: /* <名称> */
                if (isName(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_Goto(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "去" */
                if (isReserved(S_FOU_ZE_JIE_SHU, s, top))
                {
                        status = 1;
                        *matchedType = ST_GOTO;
                }
                break;
        case 1: /* <名称> */
                if (isName(s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Name>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_LoopBegin(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "重复执行" */
                if (isReserved(S_CHONG_FU_ZHI_XING, s, top))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_LOOP_BEGIN;
                }
                break;
        }
        return status;
}

static int match_LoopCheck(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "检测" */
                if (isReserved(S_JIAN_CE, s, top))
                {
                        status = 1;
                        *matchedType = ST_LOOP_CHECK;
                }
                break;
        case 1: /* <Value> | <Cell> */
                if (isValue(s, top) || isCell(s, top))
                {
                        status = 2;
                }
                else
                {
                        // Expect <Value> or <Cell>
                        wprintf(ERR_EXPECT_SYMBLE("<Value> or <Cell>"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        case 2: /* <Relational Op> | <Logical Op> | "条件"*/
                if (isRelationalOp(s, top) || isLogicalOp(s, top))
                {
                        status = 1;
                }
                else if (isReserved(S_TIAO_JIAN, s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Relational Op> or <Logical Op> or "条件"
                        wprintf(ERR_EXPECT_SYMBLE("<Relational Op> or <Logical Op> or '条件'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

static int match_LoopEnd(DgyStack *analysisStack, StatType *matchedType)
{
        static int status = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == status ||
            STATTYPE_UNDEFINED == *matchedType)
        {
                status = 0;
        }
        switch (status)
        {
        case 0: /* "直到" | "无条件" */
                if (isReserved(S_ZHI_DAO, s, top))
                {
                        status = 1;
                        *matchedType = ST_LOOP_END;
                }
                else if (isReserved(S_WU_TIAO_JIAN, s, top))
                {
                        status = MATCH_COMPLETED;
                        *matchedType = ST_LOOP_END;
                }
                break;
        case 1: /* "成立" | "不成立" */
                if (isReserved(S_CHENG_LI, s, top) ||
                    isReserved(S_BU_CHENG_LI, s, top))
                {
                        status = MATCH_COMPLETED;
                }
                else
                {
                        // Expect "成立" or "不成立"
                        wprintf(ERR_EXPECT_SYMBLE("'成立' or '不成立'"));
                        status = 0;
                        *matchedType = STATTYPE_UNDEFINED;
                }
                break;
        }
        return status;
}

ErrCode dgyDoParser(FILE *in, DgyStack *codeStack, const int maxMatchedCnt)
{
        DgyStack analysisStack;
        dgyStackInit(&analysisStack, 16); /* Create analysisStack */
        ErrCode code = CODE_SUCCESS;
        if (!in || !codeStack)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDoParser");
                return CODE_FAILURE;
        }
        int matchedCnt = 0;
        if (maxMatchedCnt == -1) /* If maxMatchedCnt == -1, set matchedCnt to a number smaller than -1. */
        {
                matchedCnt = -2;
        }
        StatType matchedType = STATTYPE_UNDEFINED;
        for (wint_t t; (matchedCnt < maxMatchedCnt) && (t = fgetwc(in)) != WEOF;)
        {
                ungetwc(t, in); /* t is a temporary variable used to detect whether EOF has been reached. */
                int matched = 0;
                while (CODE_SUCCESS == getSymble(in, codeStack, &analysisStack))
                {
                        if (matchedType == STATTYPE_UNDEFINED ||
                            matchedType == ST_OBJ_BEGIN)
                        {
                                /* Object Declaration Begin */
                                if (MATCH_COMPLETED == match_ObjBegin(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约ObjBegin\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_OBJ_END)
                        {
                                /* Object Declaration End */
                                if (MATCH_COMPLETED == match_ObjEnd(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约ObjEnd\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_MOV)
                        {
                                /* Move Value */
                                if (MATCH_COMPLETED == match_Mov(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约Mov\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_SET_REG)
                        {
                                /* Set Register */
                                if (MATCH_COMPLETED == match_SetReg(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约SetReg\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_EXEC)
                        {
                                /* Execute Object */
                                if (MATCH_COMPLETED == match_Exec(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约Exec\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_IF)
                        {
                                /* Branch Begin */
                                if (MATCH_COMPLETED == match_If(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约If\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_ELSE)
                        {
                                /* Branch Else */
                                if (MATCH_COMPLETED == match_Else(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约Else\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_ELSE_END)
                        {
                                /* Branch End */
                                if (MATCH_COMPLETED == match_ElseEnd(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约ElseEnd\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_HEREIS)
                        {
                                /* Set Label */
                                if (MATCH_COMPLETED == match_Hereis(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约Hereis\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_GOTO)
                        {
                                /* Goto Label */
                                if (MATCH_COMPLETED == match_Goto(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约Goto\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_LOOP_BEGIN)
                        {
                                /* Loop Begin */
                                if (MATCH_COMPLETED == match_LoopBegin(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约LoopBegin\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_LOOP_CHECK)
                        {
                                /* Loop Check */
                                if (MATCH_COMPLETED == match_LoopCheck(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约LoopCheck\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else if (matchedType == STATTYPE_UNDEFINED ||
                                 matchedType == ST_LOOP_END)
                        {
                                /* Loop End */
                                if (MATCH_COMPLETED == match_LoopEnd(&analysisStack, &matchedType))
                                {
                                        wprintf(L"归约LoopEnd\n");
                                        matched = 1;
                                        matchedType = STATTYPE_UNDEFINED;
                                }
                        }
                        else
                        {
                        }
                }
                // check matched
                if (matched && maxMatchedCnt != -1) /* If maxMatchedCnt == -1, never increment matchedCnt. */
                {
                        matchedCnt++;
                }
        }
        if (ferror(in))
        {
                if (errno == EILSEQ)
                        wprintf(L"Character encoding error while reading.\n");
                else
                        wprintf(L"I/O error when reading\n");
                code = CODE_FAILURE;
        }
        else if (feof(in))
        {
                if (maxMatchedCnt != -1 && matchedCnt < maxMatchedCnt)
                {
                        wprintf(L"dgyDoParser: Cannot get enough matched results\n");
                        code = CODE_FAILURE;
                }
        }
        dgyStackDestroy(&analysisStack); /* Destroy analysisStack */
        return code;
}

ErrCode fdgyDoParser(const char *fname, DgyStack *codeStack)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoParser: fopen() failed");
                return CODE_FAILURE;
        }
        code = dgyDoParser(fp, codeStack, -1);
        fclose(fp);
        return code;
}

ErrCode dgyDoParserOnce(FILE *in, DgyStack *stack)
{
        ErrCode code = CODE_FAILURE;
        code = dgyDoParser(in, stack, 1);
        return code;
}
