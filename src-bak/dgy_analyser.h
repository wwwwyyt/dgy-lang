#ifndef _dgy_analyser_h
#define _dgy_analyser_h

#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"
#include "dgy_dict.h"
#include "dgy_lexer.h"

typedef enum
{
        A_DISCARD,              /* Discard symbol */
        A_GET_VALUE,            /* Get symbol value */
        ANALYSE_OP_CNT
} AnalyseOp;

ErrCode dgyAnalyse(DgyStack *stack, Dictionary *dict, AnalyseOp op);

#endif
