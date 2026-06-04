#include "dgy_parser.h"

static ErrCode getSymble(FILE *in, DgyStack *code, DgyStack *analysis);

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

static int isValue(SymbleType type)
{
        return type == S_NAME || type == S_IMMD || type == S_CHAR;
}

static int match_Mov(DgyStack *analysisStack)
{
        static int matched = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == matched)
        {
                matched = 0;
        }
        switch (matched)                
        {
        case 0:
                if (top > 0 && s[top] == S_RESERVED && s[top - 1] == S_CUN) /* "存" */
                {
                        matched = 1;
                }
                break;
        case 1:
                if ((isValue(s[top]) || s[top] == S_CELL)) /* <Value> | <Cell> */
                {
                        matched = 2;
                }
                else
                {
                        // Expect <Value> or <Cell>
                        wprintf(ERR_EXPECT_SYMBLE("<Value> or <Cell>"));
                        matched = 0;
                }
                break;
        case 2:
                if (top > 0 && s[top] == S_RESERVED && s[top - 1] == S_DAO)
                {
                        matched = 3;
                }
                else
                {
                        // Expect "到"
                        wprintf(ERR_EXPECT_SYMBLE("到"));
                        matched = 0;
                }
                break;
        case 3:
                if (s[top] == S_CELL || s[top] == S_NAME) /* <Cell> | <Name> */
                {
                        matched = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Cell> or <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Cell> or <Name>"));
                        matched = 0;
                }
                break;
        }
        return matched;
}

static int match_SetReg(DgyStack *analysisStack)
{
        static int matched = 0;
        int top = analysisStack->sp - 1;
        cell_t *s = analysisStack->stack;
        if (MATCH_COMPLETED == matched)
        {
                matched = 0;
        }
        switch (matched)                
        {
        case 0:
                if (top > 0 && s[top] == S_RESERVED && s[top - 1] == S_SHE) /* "设" */
                {
                        matched = 1;
                }
                break;
        case 1:
                if (s[top] == S_NAME) /* <Name> */
                {
                        matched = 2;
                }
                else
                {
                        // Expect <Name>
                        wprintf(ERR_EXPECT_SYMBLE("<Name>"));
                        matched = 0;
                }
                break;
        case 2:
                if (top > 0 && s[top] == S_OP && s[top - 1] == S_EQ)
                {
                        matched = 3;
                }
                else
                {
                        // Expect "="
                        wprintf(ERR_EXPECT_SYMBLE("="));
                        matched = 0;
                }
                break;
        case 3:
                if ((isValue(s[top]) || s[top] == S_CELL)) /* <Cell> | <Value> */
                {
                        matched = MATCH_COMPLETED;
                }
                else
                {
                        // Expect <Cell> or <Value>
                        wprintf(ERR_EXPECT_SYMBLE("<Cell> or <Value>"));
                        matched = 0;
                }
                break;
        }        
        return matched;
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
        for (; (matchedCnt < maxMatchedCnt) && !feof(in);)
        {
                int matched = 0;
                while (CODE_SUCCESS == getSymble(in, codeStack, &analysisStack))
                {
                        if (MATCH_COMPLETED == match_Mov(&analysisStack))
                        {
                                wprintf(L"归约Mov\n");
                                matched = 1;
                        }
                        else if (MATCH_COMPLETED == match_SetReg(&analysisStack))
                        {
                                wprintf(L"归约SetReg\n");
                                matched = 1;
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
