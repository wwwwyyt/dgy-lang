#include "dgy_test.h"
#include "dgy_analyser.h"
#include "dgy_dict.h"
#include "dgy_parser.h"
#include "dgy_stack.h"

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

        DgyParser parser;
        dgyParserInit(&parser);

        DgyAnalyser analyser;
        dgyAnalyserInit(&analyser, &code, &word);

        dgyDoParserOnce(&parser, stdin);
        dgyStackDump(&code, -1, -1);

        dgyParserDestroy(&parser);
        dgyDictDestroy(&word);
        dgyStackDestroy(&code);
}

void dgyUnitTest(void)
{
        if (0)
        {
                test_lexer();                
        }
}
