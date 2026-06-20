#include "dgy_builtin.h"

static void (*_builtins[BUILTIN_CNT])(DgyStack *dataStack);

static void inline addBuiltin(BuiltinCode code,
                              const wchar_t *name,
                              void (*_builtin)(DgyStack *),
                              DgyStack *codeStack,
                              DgyDict *wordDict)
{
        cell_t builtin;
        builtin.type = CELL_CODE_BUILTIN_WORD;
        builtin.data.sint = code;
        _builtins[code] = _builtin;
        dgyDictAdd(wordDict, name, codeStack->sp, 0);
        dgyStackPush(codeStack, builtin);        
}

static ErrCode stackCheck(DgyStack *dataStack,
                          const wchar_t *funcName,
                          i32 argCnt)
{
        const int bp = dataStack->sp - argCnt;        
        if (bp < 0)
        {
                dgySetErr(ERR_UNDERFLOW, funcName);
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}

static void stackReturn(DgyStack *dataStack,
                           const wchar_t *funcName,
                           i32 argCnt,
                           i32 retCnt)
{
        for (i32 i = 0; i < argCnt - retCnt; ++i)
        {
                if (CODE_FAILURE == dgyStackPop(dataStack))
                {
                        wprintf(L"%ls: ", funcName);
                        dgyGetErr();       
                }
        }
}

static void sintadd(DgyStack *dataStack)
{
        i32 argCnt = 2;
        i32 retCnt = 1;
        const wchar_t *funcName = L"dgy_builtin: sintadd";
        if (CODE_SUCCESS == stackCheck(dataStack,
                                       funcName,
                                       argCnt))
        {
                cell_t *s = dataStack->stack;
                const int bp = dataStack->sp - argCnt;
                s[bp].data.sint += s[bp + 1].data.sint;
        }
        stackReturn(dataStack, funcName, argCnt, retCnt);
}

static void sintsub(DgyStack *dataStack)
{
        i32 argCnt = 2;
        i32 retCnt = 1;
        const wchar_t *funcName =  L"dgy_builtin: sintsub";
        if (CODE_SUCCESS == stackCheck(dataStack,
                                       funcName,
                                       argCnt))
        {
                cell_t *s = dataStack->stack;
                const int bp = dataStack->sp - argCnt;
                s[bp].data.sint -= s[bp + 1].data.sint;
        }
        stackReturn(dataStack, funcName, argCnt, retCnt);
}

static void sintmul(DgyStack *dataStack)
{
        i32 argCnt = 2;
        i32 retCnt = 1;
        const wchar_t *funcName = L"dgy_builtin: sintmul";
        if (CODE_SUCCESS == stackCheck(dataStack,
                                       funcName,
                                       argCnt))
        {
                cell_t *s = dataStack->stack;
                const int bp = dataStack->sp - argCnt;
                s[bp].data.sint *= s[bp + 1].data.sint;
        }
        stackReturn(dataStack, funcName, argCnt, retCnt);
}

static void sintdiv(DgyStack *dataStack)
{
        i32 argCnt = 2;
        i32 retCnt = 1;
        const wchar_t *funcName = L"dgy_builtin: sintdiv";
        if (CODE_SUCCESS == stackCheck(dataStack,
                                       funcName,
                                       argCnt))
        {
                cell_t *s = dataStack->stack;
                const int bp = dataStack->sp - argCnt;
                s[bp].data.sint /= s[bp + 1].data.sint;
        }
        stackReturn(dataStack, funcName, argCnt, retCnt);
}

ErrCode dgyBuiltinInit(DgyStack *codeStack, DgyDict *wordDict)
{
        if (!codeStack || !wordDict)
        {
                dgySetErr(ERR_NULLPTR, L"dgyBuiltinInit");
                return CODE_FAILURE;
        }
        addBuiltin(BW_SINT_ADD, L"相加", sintadd, codeStack, wordDict);
        addBuiltin(BW_SINT_SUB, L"相减", sintsub, codeStack, wordDict);
        addBuiltin(BW_SINT_MUL, L"相乘", sintmul, codeStack, wordDict);
        addBuiltin(BW_SINT_DIV, L"相整除", sintdiv, codeStack, wordDict);
        
        return CODE_SUCCESS;
}

ErrCode dgyExecBuiltin(DgyStack *dataStack, BuiltinCode code)
{
        if (!dataStack)
        {
                dgySetErr(ERR_NULLPTR, L"dgyExecBuiltin");
                return CODE_FAILURE;
        }
        _builtins[code](dataStack);
        return CODE_SUCCESS;
}
