#include "dgy_builtin.h"
#include "dgy_all.h"

static const wchar_t *_builtinWordName[BUILT_IN_WORD_CNT];
static void (*_dgyBuiltInWords[BUILT_IN_WORD_CNT])(DgyStack *stack);

void dgyNop(DgyStack *stack)
{
        // Dummy
}

void dgyAdd(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyAdd");
                return;
        }
        if (s[bp].type == T_SINT &&
            s[bp + 1].type == T_SINT)
        {
                s[bp].data.sint += s[bp + 1].data.sint;
                stack->sp = bp + retc;
        }
}

void dgyInc(DgyStack *stack)
{
        const int argc = 1;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyInc");
                return;
        }
        s[bp].data.sint++;
        stack->sp = bp + retc;
}

void dgySub(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgySub");
                return;
        }
        s[bp] -= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyDec(DgyStack *stack)
{
        const int argc = 1;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyDec");
                return;
        }
        s[bp]--;
        stack->sp = bp + retc;
}

void dgyMul(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyMul");
                return;
        }
        s[bp] *= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyDiv(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyDiv");
                return;
        }
        s[bp] /= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyAnd(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyAnd");
                return;
        }
        s[bp] &= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyOr(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyOr");
                return;
        }
        s[bp] |= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyNot(DgyStack *stack)
{
        const int argc = 1;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyNot");
                return;
        }
        s[bp] = ~s[bp];
        stack->sp = bp + retc;
}

void dgyNor(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyNor");
                return;
        }
        s[bp] ^= s[bp + 1];
        stack->sp = bp + retc;
}

void dgyFormat(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 1;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyFormat");
                return;
        }
        const wchar_t wc1 = s[bp], wc2 = s[bp + 1];
        if (wc1 == L'整' && wc2 == L'数')
                s[bp] = 0;
        else if (wc1 == L'字' && wc2 == L'符')
                s[bp] = 1;
        else if (wc1 == L'单' && wc2 == L'元')
                s[bp] = 2;
        else
                s[bp] = 2;
        stack->sp = bp + retc;
}

void dgyPrintf(DgyStack *stack)
{
        const int argc = 2;
        const int retc = 0;
        const int bp = stack->sp - argc;
        cell_t *s = stack->stack;
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyPrintf");
                return;
        }
        const int fmt = s[bp];
        const wchar_t wc = s[bp + 1];
        switch (fmt)
        {
        case 0:                 /* *整数* 格式的 */
                wprintf(L"%lld", wc);
                break;
        case 1:                 /* *字符* 格式的 */
                wprintf(L"%lc", wc);
                break;
        case 2:                 /* *单元* 格式的 */
        default:
                wprintf(L"0x%llX", wc);
                break;
        }
        stack->sp = bp + retc;
}

const wchar_t *dgyGetBuiltInWordName(BuiltInWordCode op)
{
        static int init = 0;
        if (init == 0)
        {
                init = 1;
                _builtinWordName[W_NOP] = L"无操作";
                _builtinWordName[W_ADD] = L"相加";
                _builtinWordName[W_INC] = L"加一";
                _builtinWordName[W_SUB] = L"相减";
                _builtinWordName[W_DEC] = L"减一";
                _builtinWordName[W_MUL] = L"相乘";
                _builtinWordName[W_DIV] = L"相除";
                _builtinWordName[W_AND] = L"按位与";
                _builtinWordName[W_OR] = L"按位或";
                _builtinWordName[W_NOT] = L"按位取反";
                _builtinWordName[W_NOR] = L"按位异或";
                
                _builtinWordName[W_FORMAT] = L"格式的";
                _builtinWordName[W_PRINTF] = L"打印";                
        }
        if (op >= 0 && op < BUILT_IN_WORD_CNT)
        {
                return _builtinWordName[op];
        }
        else
        {
                if (op >= BUILT_IN_WORD_CNT)
                        dgySetErr(ERR_OVERFLOW, L"dgyGetBuiltInWordName");
                else
                        dgySetErr(ERR_UNDERFLOW, L"dgyGetBuiltInWordName");
                return NULL;
        }
}

ErrCode dgyExecBuiltIn(BuiltInWordCode op, DgyStack *stack)
{
        static int init = 0;
        if (init == 0)
        {
                init = 1;
                _dgyBuiltInWords[W_NOP] = dgyNop;
                _dgyBuiltInWords[W_ADD] = dgyAdd;
                _dgyBuiltInWords[W_INC] = dgyInc;
                _dgyBuiltInWords[W_SUB] = dgySub;
                _dgyBuiltInWords[W_DEC] = dgyDec;
                _dgyBuiltInWords[W_MUL] = dgyMul;
                _dgyBuiltInWords[W_DIV] = dgyDiv;
                _dgyBuiltInWords[W_AND] = dgyAnd;
                _dgyBuiltInWords[W_OR] = dgyOr;
                _dgyBuiltInWords[W_NOT] = dgyNot;
                _dgyBuiltInWords[W_NOR] = dgyNor;
                
                _dgyBuiltInWords[W_FORMAT] = dgyFormat;
                _dgyBuiltInWords[W_PRINTF] = dgyPrintf;
        }
        if (op >= 0 && op < BUILT_IN_WORD_CNT)
        {
                _dgyBuiltInWords[op](stack);
                dgyError();
        }
        else
        {
                if (op >= BUILT_IN_WORD_CNT)
                        dgySetErr(ERR_OVERFLOW, L"dgyExecBuiltIn");
                else
                        dgySetErr(ERR_UNDERFLOW, L"dgyExecBuiltIn");
                return CODE_FAILURE;                
        }
        return CODE_SUCCESS;
}
