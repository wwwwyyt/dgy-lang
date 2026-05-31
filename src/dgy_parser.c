#include "dgy_parser.h"

ErrCode fdgyDoParser(const char *fname, DgyStack *stack)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoParser");
                return CODE_FILE_OPEN_FAIL;
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
        if (!stream)
        {
                perror("dgyDoParser");
                return CODE_NULLPTR;
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
                code = dgyDoLexerOnce(stream, buffer);
                SymbleType type = S_UNDEFINED;
                for (int i = 0; i < MAX_BUF_SIZE - 1; ++i)
                {
                        if (buffer[i + 1] == L'\0')
                        {
                                type = buffer[i];
                                buffer[i] = L'\0';
                                break;
                        }
                }
                switch (type)
                {
                case S_IMMD:
                        cell_t value = wcstoull(buffer, NULL, 0);
                        if (errno == ERANGE)
                        {
                                wprintf(L"Error: number out of ranage.\n");
                                errno = 0;
                        }
                        wprintf(L"Immd='%llu'\n", value);
                        dgyStackPush(stack, value);
                        dgyStackPush(stack, type);
                        matched = 1;
                        break;
                case S_STR:
                        wprintf(L"Str='%ls'\n", buffer);
                        for (int i = 0; buffer[i] != L'\0'; ++i)
                        {
                                dgyStackPush(stack, buffer[i]);
                        }
                        dgyStackPush(stack, type);
                        matched = 1;
                        break;
                case S_COMMENT:
                        break;
                case S_RESERVED:
                        wprintf(L"Reserved='%ls'\n", ReservedSymTable[buffer[0]].symble);
                        dgyStackPush(stack, buffer[0]);
                        dgyStackPush(stack, type);
                        matched = 1;
                        break;
                case S_OP:
                        wprintf(L"Op='%ls'\n", OpSymTable[buffer[0]].symble);
                        dgyStackPush(stack, buffer[0]);
                        dgyStackPush(stack, type);
                        matched = 1;
                        break;
                case S_NAME:
                        wprintf(L"Name='%ls'\n", buffer);
                        for (int i = 0; buffer[i] != L'\0'; ++i)
                        {
                                dgyStackPush(stack, buffer[i]);
                        }
                        dgyStackPush(stack, type);
                        matched = 1;
                        break;
                default:
                        wprintf(L"Undefined symble.\n");
                        break;
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
        if (!stream)
        {
                perror("dgyDoParserOnce");
                return CODE_NULLPTR;
        }
        code = dgyDoParser(stream, stack, 1);
        return code;
}