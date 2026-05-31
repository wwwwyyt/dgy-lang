#include "dgy_parser.h"

ErrCode fdgyDoParser(const char *fname)
{
        return CODE_SUCCESS;
}

ErrCode dgyDoParser(FILE *stream)
{
        ErrCode code = CODE_FAILURE;
        enum
        {
                MAX_BUF_SIZE = (1024 + 1),
        };
        wchar_t buffer[MAX_BUF_SIZE];
        FILE *lexerOut = fmemopen(buffer, MAX_BUF_SIZE, "w+");
        if (!lexerOut)
        {
                perror("fmemopen");
                return CODE_FAILURE;
        }
        while (!feof(stream))
        {
                rewind(lexerOut);
                code = dgyDoLexerOnce(stream, lexerOut);
                fseek(lexerOut, 0, SEEK_SET);
                for (wint_t wc; (wc = fgetwc(lexerOut)) != WEOF;)
                        wprintf(L"%lc", wc);
        }
        return code;
}
