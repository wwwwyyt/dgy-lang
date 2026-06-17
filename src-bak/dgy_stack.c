#include "dgy_stack.h"
#include "dgy_error.h"

ErrCode dgyStackInit(DgyStack *s, size_t size)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackInit");
                return CODE_FAILURE;
        }
        memset(s, 0, sizeof(DgyStack));
        s->size = size;
        s->stack = (cell_t *)malloc(s->size * sizeof(cell_t));
        if (s->stack == NULL)
        {
                perror("dgyStackInit: malloc() failed");
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}

ErrCode dgyStackResize(DgyStack *s, size_t newSize)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackResize");
                return CODE_FAILURE;
        }
        cell_t *newStack = (cell_t *)realloc(s->stack, newSize * sizeof(cell_t));
        if (newStack == NULL)
        {
                perror("dgyStackResize: realloc() failed");          
                return CODE_FAILURE;
        }
        s->stack = newStack;
        s->size = newSize;
        return CODE_SUCCESS;
}

ErrCode dgyStackPop(DgyStack *s)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackPop");
                return CODE_FAILURE;
        }        
        if (s->sp == 0)
        {
                dgySetErr(ERR_UNDERFLOW, L"dgyStackPop");
                return CODE_FAILURE;
        }
        --(s->sp);
        if (s->sp == s->size / 4 && (CODE_SUCCESS != dgyStackResize(s, s->size / 2)))
        {
                return CODE_FAILURE;
        }
        return CODE_SUCCESS;
}

ErrCode dgyStackPush(DgyStack *s, cell_t data)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackPush");
                return CODE_FAILURE;
        }        
        if (s->sp == s->size / 2 && (CODE_SUCCESS != dgyStackResize(s, 2 * s->size)))
        {
                return CODE_FAILURE;
        }
        s->stack[(s->sp)++] = data;
        return CODE_SUCCESS;
}

ErrCode dgyStackTop(const DgyStack *s, cell_t *data)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackTop");
                return CODE_FAILURE;
        }        
        if (s->sp == 0)
        {
                return CODE_FAILURE;
        }
        *data = s->stack[s->sp - 1];
        return CODE_SUCCESS;
}

bool dgyStackEmpty(const DgyStack *s)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackEmpty");
                return true;
        }
        return s->sp == 0;
}

ErrCode dgyStackDestroy(DgyStack *s)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackDestroy");
                return CODE_FAILURE;
        }        
        free(s->stack);
        memset(s, 0, sizeof(DgyStack));
        return CODE_SUCCESS;
}

ErrCode dgyStackDump(DgyStack *s, int start, int end)
{
        if (!s)
        {
                dgySetErr(ERR_NULLPTR, L"dgyStackDump");
                return CODE_FAILURE;
        }
        if (start == -1 && end == -1)
        {
                for (int i = 0; i < s->sp; ++i)
                {
                        wprintf(L"0x%llX ", s->stack[i]);
                }                
        }
        else
        {
                for (int i = start; i < end; ++i)
                {
                        if (i >= 0 && i < s->size)
                        {
                                wprintf(L"0x%llX ", s->stack[i]);
                        }
                        else
                        {

                                dgySetErr(ERR_OUT_OF_BOUNDS, L"dgyStackDump");
                                return CODE_FAILURE;
                        }
                }
        }
        wprintf(L"\n");
        return CODE_SUCCESS;
}
