#include "dgy_object.h"

void dgyAdd(cell_t *stack, int bp, int *sp)
{
        stack[bp] += stack[bp + 1];
        *sp = bp + 1;
}

void dgyInc(cell_t *stack, int bp, int *sp)
{
        stack[bp] += 1;
        *sp = bp + 1;
}

void dgySub(cell_t *stack, int bp, int *sp)
{
        stack[bp] -= stack[bp + 1];
        *sp = bp + 1;
}

void dgyDec(cell_t *stack, int bp, int *sp)
{
        stack[bp] -= 1;
        *sp = bp + 1;
}

void dgyMul(cell_t *stack, int bp, int *sp)
{
        stack[bp] *= stack[bp + 1];
        *sp = bp + 1;
}

void dgyDiv(cell_t *stack, int bp, int *sp)
{
        stack[bp] /= stack[bp + 1];
        *sp = bp + 1;
}

void dgyGetFormat(cell_t *stack, int bp, int *sp)
{
        stack[++bp] = 0;
        *sp = bp + 1;
}

void dgyPrintf(cell_t *stack, int bp, int *sp)
{
        wchar_t *fmt[] = {
            L"%llu ",
            L"%lld ",
            L"%lc",
        };
        int fmtIdx = 0;
        switch (stack[bp])
        {
        case L'U':
                fmtIdx = 0;
                break;
        case L'D':
                fmtIdx = 1;
                break;
        case L'S':
                fmtIdx = 2;
                break;
        }
        for (int i = bp + 2; i < *sp; ++i)
        {
                wprintf(fmt[fmtIdx], stack[i]);
        }
        *sp = bp;
}

void dgyPrintStack(cell_t *stack, int bp, int *sp)
{
        wprintf(L"(%d, %d) ", bp, *sp);
        for (int i = bp; i < *sp; ++i)
        {
                wprintf(L"%llu ", stack[i]);
        }
        wprintf(L"\n");
}