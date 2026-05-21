#include "dm_core.h"

static cell_t *dstack = NULL;
static int dsp = 0;
static size_t dsize = 0;

static ErrCode init()
{
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
    return CODE_SUCCESS;
}

static ErrCode push(cell_t data)
{
    if (dsp == dsize && (CODE_SUCCESS != resize(2 * dsize)))
    {
        return CODE_OVERFLOW;
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

static void dmMov(cell_t *src, cell_t *dst)
{
    *dst = *src;
}

static void dmLet(int bp,
                  int sp,
                  void (*dmObject)(cell_t *dstack, unsigned int dbp))
{
}

ErrCode dmDo()
{
    setlocale(LC_ALL, "zh_CN.utf8");
    init();
    return CODE_SUCCESS;
}

ErrCode dmUnitTest()
{
    int i = dsp;
    push(L'S');
    dmGetFormat(dstack, i, &dsp);
    push(L'你');
    push(L'好');
    push(L'世');
    push(L'界');
    push(L'！');
    push(L'\n');
    dmPrintStack(dstack, i, &dsp);
    dmPrintf(dstack, i, &dsp);
    dmPrintStack(dstack, i, &dsp);

    return CODE_SUCCESS;
}