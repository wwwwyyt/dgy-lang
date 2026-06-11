#include "dgy_test.h"

static void getSymbol(FILE *in, FILE *out)
{
        enum
        {
                MAX_BUF_SIZE = MAX_WORD_LEN + 2
        };
        static wchar_t buffer[MAX_BUF_SIZE];
        dgyDoLexerOnce(in, buffer);
        int len = wcslen(buffer);
        SymbolType type = buffer[len - 1];
        buffer[len - 1] = L'\0';
        if (out)
        {
                fwprintf(out, L"%ls (%d)\n", buffer, type);
        }
}

static void test_lexer(const char *fname, FILE *out)
{
        FILE *in = fopen(fname, "r");
        if (!in)
        {
                perror("dgy_test: test_lexer: fopen() failed");
        }
        wint_t wc;
        for (int i = 1; (wc = fgetwc(in)) != WEOF; ++i)
        {
                if (iswspace(wc))
                        continue;
                ungetwc(wc, in);
                if (out)
                {
                        fwprintf(out, L"Test %d: ", i);
                }
                getSymbol(in, out);
        }
}


static void test_file(const char *fname)
{
        DgyStack codeStack;
        dgyStackInit(&codeStack, 16);
        fdgyDoParser(fname, &codeStack);
}

static void test_lexer_immd(FILE *out)
{
        const char *fname = "test/test_lexer_immd.dgy";
        test_lexer(fname, out);
}

static void test_lexer_str(FILE *out)
{
        const char *fname = "test/test_lexer_str.dgy";
        test_lexer(fname, out);
}

static void test_lexer_reserved(FILE *out)
{
        const char *fname = "test/test_lexer_reserved.dgy";
        test_lexer(fname, out);
}

static void test_lexer_op(FILE *out)
{
        const char *fname = "test/test_lexer_op.dgy";
        test_lexer(fname, out);
}

static void test_lexer_cell(FILE *out)
{
        const char *fname = "test/test_lexer_cell.dgy";
        test_lexer(fname, out);
}

static void test_parser(void)
{
        test_file("test/test_parser.dgy");
}

ErrCode dgyUnitTest(void)
{
        // dgyTestDo();
        enum
        {
                DONOT_TEST = 0
        };
        if (DONOT_TEST)
        {
                test_lexer_immd(stdout);
                test_lexer_str(stdout);
                test_lexer_reserved(stdout);
                test_lexer_op(stdout);
                test_lexer_cell(stdout);
                test_parser();
                dgyTestDo();
        }

        /* test_lexer_immd(stdout); */
        /* test_lexer_str(stdout); */
        /* test_lexer_reserved(stdout); */
        /* test_lexer_op(stdout); */
        /* test_lexer_cell(stdout); */
        
        // test_file("test/test_parser_err.dgy");
        dgyTestDo();
        
        return CODE_SUCCESS;
}
