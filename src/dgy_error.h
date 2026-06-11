#ifndef _dgy_error_h
#define _dgy_error_h

#include "dgy_all.h"

#define ERR_UNCLOSED_SYMBOL(sym)                \
        L"Error: Unclosed symbol: '" sym "'\n"
#define ERR_INVALID_SYMBOL(sym)                 \
        L"Error: Invalid symbol: '" sym "'\n"
#define ERR_EXPECT_SYMBOL(sym)                  \
        L"Error: Expect symbol: '" sym "'\n"
#define WARN_UNDEFINED_WORD(word)                       \
        L"Warning: Undefined word: '%ls'\n", word
#define WARN_OUT_OF_BUFFER(len)                                 \
        L"Warning: Out of buffer. Exceeding length: %d\n", len

typedef enum
{
        CODE_SUCCESS = 0,
        CODE_FAILURE = 1,
        ERR_NULLPTR = 2,
        ERR_UNDERFLOW = 3,
        ERR_OVERFLOW = 4,
        ERR_OUT_OF_BOUNDS = 5,
        ERR_CNT,
} ErrCode;

void dgySetErr(ErrCode code, const wchar_t *msg);
void dgyError(void);
void dgyPrintErrPos(wchar_t *symbol, int symbolLen, const char *fname, int row, int col);

#endif /* _dgy_error_h */
