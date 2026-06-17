#include "dgy_instr.h"
#include "dgy_error.h"

static ErrCode oneOpIns(DgyStack *stack, u32 retCnt, const wchar_t *insName)
{
        const int bp = stack->sp - opCnt;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, insName);
                return CODE_FAILURE;
        }
        switch ()
        if (s[bp].type == T_SINT &&
            s[bp + 1].type == T_SINT)
        {
                s[bp].data.sint += s[bp + 1].data.sint;
                stack->sp = bp + retCnt;
        }        
        return CODE_SUCCESS;
}
