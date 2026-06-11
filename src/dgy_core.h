#ifndef _dgy_core_h
#define _dgy_core_h

#include "dgy_all.h"
#include "dgy_builtin.h"
#include "dgy_parser.h"
#include "dgy_stack.h"
#include "dgy_dict.h"

typedef struct
{
        DgyStack dataStack;
        DgyStack regStack;
        DgyStack codeStack;
        Dictionary wordDict;
} Core;

ErrCode dgyDo(void);
ErrCode dgyTestDo(void);
void dgyQuit(void);

#endif /* _dgy_core_h */
