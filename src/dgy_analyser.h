#ifndef _dgy_analyser_h
#define _dgy_analyser_h

#include "dgy_all.h"
#include "dgy_error.h"
#include "dgy_stack.h"
#include "dgy_dict.h"
#include "dgy_lexer.h"

typedef enum
{
        STATTYPE_UNDEFINED = -1,
        ST_WORD_BEGIN = 0,
        ST_WORD_END,
        ST_MOV,
        ST_SET_REG,
        ST_EXEC,
        ST_IF,
        ST_ELSE,
        ST_ELSE_END,
        ST_HEREIS,
        ST_GOTO,
        ST_LOOP_BEGIN,
        ST_LOOP_CHECK,
        ST_LOOP_END,
        STATTYPE_CNT,
} StatType;

typedef struct
{
        DgyStack *codeStack;
        DgyDict *wordDict;
} DgyAnalyser;

/* Append data to codeStack */
ErrCode dgyAppendData(DgyAnalyser *analyser, DgyStack *symbolStack);

/* Append addresses to codeStack */
ErrCode dgyAppendAddr(DgyAnalyser *analyser, DgyStack *symbolStack);

/* Append instructions to codeStack */
ErrCode dgyAppendInstr(DgyAnalyser *analyser, StatType statType);

/* Discard the newly added content and
   restore the codeStack to its state before analysis. */
ErrCode dgyDiscard(DgyAnalyser *analyser);

ErrCode dgyAnalyserInit(DgyAnalyser *analyser,
                        DgyStack *codeStack,
                        DgyDict *wordDict);

#endif
