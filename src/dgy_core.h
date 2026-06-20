#ifndef _dgy_core_h
#define _dgy_core_h

#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"
#include "dgy_dict.h"
#include "dgy_parser.h"
#include "dgy_analyser.h"
#include "dgy_builtin.h"

typedef struct
{
        DgyStack codeStack;
        DgyStack dataStack;
        DgyStack execStack;
        DgyStack regStack;
        DgyDict wordDict;
} DgyCore;

#endif
