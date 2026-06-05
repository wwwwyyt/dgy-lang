#include "dgy_core.h"

static DgyStack dataStack;
static DgyStack codeStack;
static Dictionary nameDict;

static ErrCode nameDictInit(void)
{
        ErrCode code = dgyDictInit(&nameDict, 16);
        if (code != CODE_SUCCESS)
                return code;

        code = dgyDictAdd(L"相加", 0, &nameDict);
        if (code != CODE_SUCCESS)
                return code;

        code = dgyDictAdd(L"相减", 1, &nameDict);
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
        code = dgyStackInit(&dataStack, 16);
        if (code != CODE_SUCCESS)
                return code;

        /* Initialize code stack */
        code = dgyStackInit(&codeStack, 16);
        if (code != CODE_SUCCESS)
                return code;

        /* Initialize name dictionary */
        code = nameDictInit();
        if (code != CODE_SUCCESS)
                return code;

        return code;
}

static ErrCode pop()
{
        return dgyStackPop(&dataStack);
}

static ErrCode push(cell_t data)
{
        return dgyStackPush(&dataStack, data);
}

static cell_t top()
{
        return dgyStackTop(&dataStack);
}

static void dgyMov(cell_t *src, cell_t *dst)
{
        *dst = *src;
}

static void dgyLet(int bp,
                   int *sp,
                   void (*dgyEval)(cell_t *dstack, int bp, int *sp))
{
        dgyEval(dataStack.stack, bp, sp);
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
                code = dgyDoParserOnce(stdin, &codeStack);
                dgyPrintStack(codeStack.stack, 0, &(codeStack.sp));
        }
        return code;
}
