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
                        dgyStackPush(analysisStack, buffer[i]); 
                        break;
                }
        }
        return code;
}

static int isValue(SymbleType type)
{
        return type == S_NAME || type == S_IMMD || type == S_CHAR;
}

static int isCell(SymbleType type)
{
        return type == S_NAME || type == S_IMMD;
}

static int match_Mov(DgyStack *analysisStack)
{
        int matched = 0;
        int top = analysisStack->sp;
        cell_t *s = analysisStack->stack;
        if (matched == 0 && s[top] == S_RESERVED && s[top - 1] == S_CUN) /* "存" */
                matched = 1;
        if (matched == 1 && (isValue(s[top]) || isCell())) /* <Value> | <Cell> */
                matched = 2;
        if (matched == 2 && s[top] == S_RESERVED && s[top - 1] == S_DAO) /* "到" */
                matched = 3;
        if (matched == 3 && (isCell() || s[top] == S_NAME)) /* <Cell> | <Name> */
                matched = 4;
        return matched;
}

static int match_SetReg(DgyStack *analysisStack)
{
        int matched = 0;
        int top = analysisStack->sp;
        cell_t *s = analysisStack->stack;
        if (top > 1 && s[0] == S_RESERVED && s[1] == S_SHE) /* "设" */
                matched = 1;
        if (top > 2 && s[2] == S_NAME) /* <Name> */
                matched = 2;
        if (top > 4 && s[3] == S_OP && s[4] == S_EQ) /* "=" */
                matched = 3;
        if (top > 5 && isValue(s[5])) /* <Value> */
                matched = 4;
        return matched;
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
                        match_Mov(&analysisStack);
                        match_SetReg(&analysisStack);
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
                if (matchedCnt < maxMatchedCnt)
                {
                        wprintf(L"dgyDoParser: Cannot get enough matched results\n");
                        code = CODE_FAILURE;
                }
        }
        dgyStackDestroy(&analysisStack); /* Destroy analysisStack */
        return code;
}

ErrCode dgyDoParserOnce(FILE *in, DgyStack *stack)
{
        ErrCode code = CODE_FAILURE;
        code = dgyDoParser(in, stack, 1);
        return code;
}
