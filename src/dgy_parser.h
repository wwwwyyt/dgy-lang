#ifndef _dgy_parser_h
#define _dgy_parser_h

#include "dgy_all.h"
#include "dgy_lexer.h"
#include "dgy_stack.h"
#include "dgy_analyser.h"

enum
{
        MATCH_COMPLETED = -1
};

typedef struct
{
        DgyStack analyStack, symbolStack;
        DgyAnalyser analyser;
} DgyParser;

ErrCode dgyParserInit(DgyParser *parser,
                      DgyStack *codeStack,
                      DgyDict *wordDict);
ErrCode dgyParserDestroy(DgyParser *parser);
ErrCode dgyDoParserOnce(DgyParser *parser, FILE *in);

#endif /* _dgy_parser_h */
