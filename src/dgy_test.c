#include "dgy_test.h"
#include "dgy_builtin.h"

static void test_lexer(void)
{
        DgyStack buffer;
        dgyStackInit(&buffer, 16);
        dgyDoLexerOnce(stdin, &buffer);
        dgyStackDump(&buffer, -1, -1);        
}

static void test_parser(void)
{
        DgyStack code;        
        dgyStackInit(&code, 16);

        DgyDict word;
        dgyDictInit(&word, 16);

        dgyBuiltinInit(&code, &word);

        DgyParser parser;
        dgyParserInit(&parser, &code, &word);
        
        dgyDoParserOnce(&parser, stdin);
        dgyStackDump(&code, BUILTIN_CNT, -1);
        dgyDoParserOnce(&parser, stdin);
        dgyStackDump(&code, BUILTIN_CNT, -1);

        dgyParserDestroy(&parser);
        dgyDictDestroy(&word);
        dgyStackDestroy(&code);
}

void dgyUnitTest(void)
{
        if (0)
        {
                test_lexer();
                test_parser();
        }
        test_parser();                                
}
