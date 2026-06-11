#ifndef _dgy_parser_h
#define _dgy_parser_h

#include "dgy_all.h"
#include "dgy_lexer.h"
#include "dgy_stack.h"
#include "dgy_dict.h"

enum
{
        MATCH_COMPLETED = -1
};

typedef enum
{
        STATTYPE_UNDEFINED,
        ST_WORD_BEGIN,
        ST_WORD_END,
        ST_MOV,
        ST_SET_REG,
        ST_EXEC,
        ST_IF,
        ST_ELSE,
        ST_ELSE_END,
        ST_HEREIS,
        ST_GOTO,
        ST_LOOP_BEGIN,
        ST_LOOP_CHECK,
        ST_LOOP_END,
        STATTYPE_END,
} StatType;

ErrCode fdgyDoParser(const char *fname, DgyStack *stack);
ErrCode dgyDoParser(FILE *stream, DgyStack *stack, const int maxMatchedCnt);
ErrCode dgyDoParserOnce(FILE *stream, DgyStack *stack);

#endif /* _dgy_parser_h */
