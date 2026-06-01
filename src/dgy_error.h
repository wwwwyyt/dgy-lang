#ifndef _dgy_error_h
#define _dgy_error_h

#include "dgy_all.h"

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
