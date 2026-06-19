#include "dgy_analyser.h"
#include "dgy_error.h"

ErrCode dgyAppendData(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        return CODE_SUCCESS;
}

ErrCode dgyAppendAddr(DgyAnalyser *analyser, DgyStack *symbolStack)
{
        return CODE_SUCCESS;
}

ErrCode dgyAppendInstr(DgyAnalyser *analyser, StatType statType)
{
        return CODE_SUCCESS;
}

ErrCode dgyDiscard(DgyAnalyser *analyser)
{
        return CODE_SUCCESS;
}

ErrCode dgyAnalyserInit(DgyAnalyser *analyser,
                        DgyStack *codeStack,
                        DgyDict *wordDict)
{
        if (!analyser || !codeStack || !wordDict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyAnalyserInit");
                return CODE_FAILURE;
        }
        analyser->codeStack = codeStack;
        analyser->wordDict = wordDict;
        return CODE_SUCCESS;
}
