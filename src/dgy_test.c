#include "dgy_test.h"

static void getSymble(FILE *in, FILE *out)
{
        enum
        {
                MAX_BUF_SIZE = MAX_NAME_LEN + 2
        };
        static wchar_t buffer[MAX_BUF_SIZE];
        dgyDoLexerOnce(in, buffer);
        int len = wcslen(buffer);
        SymbleType type = buffer[len - 1];
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
                getSymble(in, out);
        }
}

static void test_lexer_immd(FILE *out)
{
        const char *fname = "dgy/test_lexer_immd.dgy";
        test_lexer(fname, out);
}

static void test_lexer_str(FILE *out)
{
        const char *fname = "dgy/test_lexer_str.dgy";
        test_lexer(fname, out);
}

static void test_lexer_reserved(FILE *out)
{
        const char *fname = "dgy/test_lexer_reserved.dgy";
        test_lexer(fname, out);
}

static void test_lexer_op(FILE *out)
{
        const char *fname = "dgy/test_lexer_op.dgy";
        test_lexer(fname, out);
}

static void test_lexer_cell(FILE *out)
{
        const char *fname = "dgy/test_lexer_cell.dgy";
        test_lexer(fname, out);
}

static void test_parser(void)
{
        DgyStack codeStack;
        dgyStackInit(&codeStack, 16);
        fdgyDoParser("dgy/test_parser.dgy", &codeStack);
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

                test_parser();

                dgyTestDo();
        }

        test_lexer_cell(stdout);

        return CODE_SUCCESS;
}
