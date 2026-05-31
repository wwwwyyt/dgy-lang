#include "dgy_core.h"

static DgyStack dataStack;

static ErrCode init()
{
        ErrCode code = CODE_FAILURE;

        /* Set locale */
        setlocale(LC_ALL, "zh_CN.utf8");

        /* Initialize data stack */
        code = dgyStackInit(&dataStack, 16);

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
