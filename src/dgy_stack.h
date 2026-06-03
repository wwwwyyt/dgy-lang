#ifndef _dgy_stack_h
#define _dgy_stack_h

#include "dgy_all.h"
#include "dgy_error.h"

typedef struct
{
        cell_t *stack;
        int sp;
        size_t size;
} DgyStack;

ErrCode dgyStackInit(DgyStack *s, size_t size);
ErrCode dgyStackResize(DgyStack *s, size_t newSize);
ErrCode dgyStackPop(DgyStack *s);
ErrCode dgyStackPush(DgyStack *s, cell_t data);
cell_t dgyStackTop(DgyStack *s);
void dgyStackDestroy(DgyStack *s);

#endif /* _dgy_stack_h */
