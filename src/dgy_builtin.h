#ifndef _dgy_builtin_h
#define _dgy_builtin_h

#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"
#include "dgy_dict.h"

typedef enum
{
        BW_SINT_ADD = 0,
        BW_SINT_SUB,
        BW_SINT_MUL,
        BW_SINT_DIV,
        BUILTIN_CNT
} BuiltinCode;

ErrCode dgyBuiltinInit(DgyStack *codeStack, DgyDict *wordDict);
ErrCode dgyExecBuiltin(DgyStack *dataStack, BuiltinCode code);

#endif
