#include "dgy_parser.h"

static ErrCode getSymble(FILE *in, wchar_t *buffer, SymbleType *type, const size_t bufSize);
static int match_SimpObj(FILE *in, wchar_t *buffer, const size_t bufSize);

static ErrCode getSymble(FILE *in, wchar_t *buffer, SymbleType *type, const size_t bufSize)
{
        ErrCode code = dgyDoLexerOnce(in, buffer);
        if (code != CODE_SUCCESS)
                return code;
        for (int i = 0; i < bufSize - 1; ++i)
        {
                if (buffer[i + 1] == L'\0')
                {
                        *type = buffer[i];
                        buffer[i] = L'\0';
                        break;
                }
        }
        return code;
}

static int match_value(FILE *in, wchar_t *buffer, const size_t bufSize)
{
        int matched = 0;
        
        return matched;
}

static int match_cell(FILE *in, wchar_t *buffer, const size_t bufSize)
{
        int matched = 0;
        return matched;        
}

static int match_valuecell(FILE *in, wchar_t *buffer, const size_t bufSize)
{
        int matched = 0;
        return matched;
}

static int match_SimpObj(FILE *in, wchar_t *buffer, const size_t bufSize)
{
        enum
        {
                START,
                STAT_1,
                STAT_2,
                END,
        };
        int matched = 0;
        SymbleType type = S_UNDEFINED;
        int status = START;
        while (CODE_SUCCESS == getSymble(in, buffer, &type, bufSize))
        {
                switch (status)
                {
                case START:
                        if (type == S_NAME)
                        {
                                status = STAT_1;
                        }
                        break;
                case STAT_1:
                        if (type == S_OP && buffer[0] == S_EQ)
                        {
                                status = STAT_2;
                        }
                        break;
                case STAT_2:
                        if (match_valuecell(in, buffer, bufSize))
                        break;
                case END:
                        break;
                }
        }
        return matched;
}

ErrCode fdgyDoParser(const char *fname, DgyStack *stack)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoParser: fopen() failed");
                return CODE_FAILURE;
        }
        code = dgyDoParser(fp, stack, -1);
        fclose(fp);
        return code;
}

ErrCode dgyDoParser(FILE *stream, DgyStack *stack, const int maxMatchedCnt)
{
        enum
        {
                MAX_BUF_SIZE = MAX_NAME_LEN + 2,
        };
        ErrCode code = CODE_FAILURE;
        if (!stream || !stack)
        {
                dgySetErr(ERR_NULLPTR, L"dgyDoParser");
                return code;
        }
        wchar_t buffer[MAX_BUF_SIZE];
        int matchedCnt = 0;
        if (maxMatchedCnt == -1) /* If maxMatchedCnt == -1, set matchedCnt to a number smaller than -1. */
        {
                matchedCnt = -2;
        }
        for (; (matchedCnt < maxMatchedCnt) && !feof(stream);)
        {
                int matched = 0;
                if (match_SimpObj(stream, buffer, MAX_BUF_SIZE))
                {
                        matched = 1;
                }
                else
                {
                        
                }
                
                // check matched
                if (matched && maxMatchedCnt != -1) /* If maxMatchedCnt == -1, never increment matchedCnt. */
                {
                        matchedCnt++;
                }
        }
        return code;
}

ErrCode dgyDoParserOnce(FILE *stream, DgyStack *stack)
{
        ErrCode code = CODE_FAILURE;
        code = dgyDoParser(stream, stack, 1);
        return code;
}
