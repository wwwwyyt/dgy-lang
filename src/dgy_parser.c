#include "dgy_parser.h"

ErrCode fdgyDoParser(const char *fname)
{
        ErrCode code = CODE_FAILURE;
        FILE *fp = fopen(fname, "r");
        if (!fp)
        {
                perror("fdgyDoParser");
                return CODE_FILE_OPEN_FAIL;
        }
        code = dgyDoParser(fp);
        fclose(fp);
        return code;
}

ErrCode dgyDoParser(FILE *stream)
{
        ErrCode code = CODE_FAILURE;
        enum
        {
                MAX_BUF_SIZE = MAX_NAME_LEN + 2,
        };
        wchar_t buffer[MAX_BUF_SIZE];
        while (!feof(stream))
        {
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
                        break;
                case S_STR:
                        wprintf(L"Str='%ls'\n", buffer);
                        break;
                case S_COMMENT:
                        break;
                case S_RESERVED:
                        wprintf(L"Reserved='%ls'\n", ReservedSymTable[buffer[0]].symble);
                        break;
                case S_OP:
                        wprintf(L"Op='%ls'\n", OpSymTable[buffer[0]].symble);
                        break;
                case S_NAME:
                        wprintf(L"Name='%ls'\n", buffer);
                        break;
                default:
                        wprintf(L"Undefined symble.\n");
                        break;
                }
        }
        return code;
}
