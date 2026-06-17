#ifndef _dgy_stack_h
#define _dgy_stack_h

#include "dgy_all.h"
#include "dgy_error.h"

typedef struct
{
        cell_t *stack;
        i32 sp;
        size_t size;
} DgyStack;

ErrCode dgyStackInit(DgyStack *s, size_t size);
ErrCode dgyStackResize(DgyStack *s, size_t newSize);
ErrCode dgyStackPop(DgyStack *s);
ErrCode dgyStackPush(DgyStack *s, cell_t data);
ErrCode dgyStackTop(const DgyStack *s, cell_t *data);
bool dgyStackEmpty(const DgyStack *s);
ErrCode dgyStackDestroy(DgyStack *s);
ErrCode dgyStackDump(DgyStack *s, i32 start, i32 end);

#endif /* _dgy_stack_h */
