#ifndef _dgy_error_h
#define _dgy_error_h

#include "dgy_all.h"

#define ERR_UNCLOSED_SYMBLE(sym) \
        L"Error: Unclosed symble: '" sym "'\n"
#define ERR_INVALID_SYMBLE(sym) \
        L"Error: Invalid symble: '" sym "'\n"
#define ERR_EXPECT_SYMBLE(sym) \
        L"Error: Expect symble: '" sym "'\n"
#define WARN_OUT_OF_BUFFER(len) \
        L"Warning: Out of buffer. Exceeding length: %d\n", len

typedef enum
{
        CODE_SUCCESS = 0,
        CODE_FAILURE = 1,
        ERR_NULLPTR = 2,
        ERR_UNDERFLOW = 3,
        ERR_CNT,
} ErrCode;

void dgySetErr(ErrCode code, const wchar_t *msg);
void dgyError(void);

#endif /* _dgy_error_h */
