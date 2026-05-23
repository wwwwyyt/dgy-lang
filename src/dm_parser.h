#ifndef _dm_parser_h
#define _dm_parser_h

#include "dm_all.h"

#define ERR_UNCLOSED_SYMBLE(sym) L"\nError: Unclosed symble: '" sym "'\n"
#define ERR_INVALID_SYMBLE(sym) L"\nError: Invalid symble: '" sym "'\n"

ErrCode fdmDoLexer(const char *fname);

#endif /* _dm_parser_h */