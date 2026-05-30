#include "dgy_core.h"

static cell_t *dstack = NULL;
static int dsp = 0;
static size_t dsize = 0;

static ErrCode init()
{
        /* Set locale */
        setlocale(LC_ALL, "zh_CN.utf8");

        /* Initialize data stack */
        dsize = 16;
        dstack = (cell_t *)malloc(dsize * sizeof(cell_t));

        return CODE_SUCCESS;
}

static ErrCode resize(size_t newSize)
{
        cell_t *newStack = (cell_t *)realloc(dstack, newSize * sizeof(cell_t));
        if (newStack == NULL)
        {
                return CODE_ALLOC_FAIL;
        }
        dstack = newStack;
        dsize = newSize;
        return CODE_SUCCESS;
}

static ErrCode pop()
{
        if (dsp == 0)
        {
                return CODE_UNDERFLOW;
        }
        --dsp;
        if (dsp == dsize / 4 && (CODE_SUCCESS != resize(dsize / 2)))
        {
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}

static ErrCode push(cell_t data)
{
        if (dsp == dsize / 2 && (CODE_SUCCESS != resize(2 * dsize)))
        {
                return CODE_FAILURE;
        }
        dstack[dsp++] = data;
        return CODE_SUCCESS;
}

static cell_t top()
{
        if (dsp == 0)
        {
                return 0;
        }
        return dstack[dsp - 1];
}

static void dgyMov(cell_t *src, cell_t *dst)
{
        *dst = *src;
}

static void dgyLet(int bp,
                   int *sp,
                   void (*dgyEval)(cell_t *dstack, int bp, int *sp))
{
        dgyEval(dstack, bp, sp);
}

ErrCode dgyDo()
{
        init();
        return CODE_SUCCESS;
}
