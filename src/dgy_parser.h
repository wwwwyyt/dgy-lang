#ifndef _dgy_parser_h
#define _dgy_parser_h

#include "dgy_all.h"
#include "dgy_lexer.h"
#include "dgy_stack.h"
#include "dgy_dict.h"

enum { MATCH_COMPLETED = -1 };

ErrCode fdgyDoParser(const char *fname, DgyStack *stack);
ErrCode dgyDoParser(FILE *stream, DgyStack *stack, const int maxMatchedCnt);
ErrCode dgyDoParserOnce(FILE *stream, DgyStack *stack);

#endif /* _dgy_parser_h */
