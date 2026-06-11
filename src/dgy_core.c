#include "dgy_core.h"

static DgyStack _dataStack;
static DgyStack _codeStack;
static Dictionary _nameDict;

static ErrCode nameDictInit(void)
{
        ErrCode code = dgyDictInit(&_nameDict, 16);
        if (code != CODE_SUCCESS)
                return code;

        code = dgyDictAdd(L"相加", 0, &_nameDict);
        if (code != CODE_SUCCESS)
                return code;

        code = dgyDictAdd(L"相减", 1, &_nameDict);
        if (code != CODE_SUCCESS)
                return code;

        return code;
}

static ErrCode init()
{
        ErrCode code = CODE_FAILURE;

        /* Set locale */
        setlocale(LC_ALL, "zh_CN.utf8");

        /* Initialize data stack */
        code = dgyStackInit(&_dataStack, 16);
        if (code != CODE_SUCCESS)
                return code;

        /* Initialize code stack */
        code = dgyStackInit(&_codeStack, 16);
        if (code != CODE_SUCCESS)
                return code;

        /* Initialize name dictionary */
        code = nameDictInit();
        if (code != CODE_SUCCESS)
                return code;

        return code;
}

static ErrCode pop(void)
{
        return dgyStackPop(&_dataStack);
}

static ErrCode push(cell_t data)
{
        return dgyStackPush(&_dataStack, data);
}

static cell_t top(void)
{
        return dgyStackTop(&_dataStack);
}

static void dgyMov(cell_t *src, cell_t *dst)
{
        *dst = *src;
}

static void dgyLet(int bp,
                   int *sp,
                   void (*dgyEval)(cell_t *dstack, int bp, int *sp))
{
        dgyEval(_dataStack.stack, bp, sp);
}

ErrCode dgyDo()
{
        init();
        return CODE_SUCCESS;
}

ErrCode dgyTestDo()
{
        ErrCode code = CODE_FAILURE;
        while (1)
        {
                code = dgyDoParserOnce(stdin, &_codeStack);
                dgyPrintStack(_codeStack.stack, 0, &(_codeStack.sp));
        }
        return code;
}

void dgyQuit(void)
{
        dgyStackDestroy(&_dataStack);
        dgyStackDestroy(&_codeStack);
        dgyDictDestroy(&_nameDict);
        
}
