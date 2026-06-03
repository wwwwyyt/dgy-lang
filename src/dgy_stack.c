#include "dgy_stack.h"

ErrCode dgyStackInit(DgyStack *s, size_t size)
{
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
        if (s->sp == s->size / 2 && (CODE_SUCCESS != dgyStackResize(s, 2 * s->size)))
        {
                return CODE_FAILURE;
        }
        s->stack[(s->sp)++] = data;
        return CODE_SUCCESS;
}

cell_t dgyStackTop(DgyStack *s)
{
        if (s->sp == 0)
        {
                return 0;
        }
        return s->stack[s->sp - 1];
}

void dgyStackDestroy(DgyStack *s)
{
        free(s->stack);
        memset(s, 0, sizeof(DgyStack));
}
