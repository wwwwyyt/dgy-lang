#ifndef _dgy_parser_h
#define _dgy_parser_h

#include "dgy_all.h"
#include "dgy_lexer.h"
#include "dgy_stack.h"

ErrCode fdgyDoParser(const char *fname);
ErrCode dgyDoParser(FILE *stream);

#endif /* _dgy_parser_h */