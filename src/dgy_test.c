#include "dgy_test.h"

static void test_lexer(void)
{
        DgyStack buffer;
        dgyStackInit(&buffer, 16);
        dgyDoLexerOnce(stdin, &buffer);
        dgyStackDump(&buffer, -1, -1);
}

static void test_parser(void)
{
        DgyParser parser;
        dgyParserInit(&parser);
        dgyDoParserOnce(&parser, stdin);
        dgyStackDump(&(parser.symbolStack), 0, -1);
        dgyParserDestroy(&parser);
}

static void test_analyser(void)
{
}

void dgyUnitTest(void)
{
        if (0)
        {
                test_lexer();
                test_parser();
                test_analyser();
        }
        test_analyser();
}
